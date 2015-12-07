// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "evita/views/metrics_view.h"

#include "base/bind.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/text_format.h"
#include "evita/gfx/text_layout.h"
#include "evita/metrics/sampling.h"
#include "evita/paint/paint_thread.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/compositor/layer_owner.h"

namespace views {

namespace {

const size_t kNumberOfSamples = 200;
const int kViewHeight = 100;
const int kViewWidth = 250;

void PaintSamples(gfx::Canvas* canvas,
                  const gfx::Brush& brush,
                  const gfx::RectF& bounds,
                  const metrics::Sampling& data) {
  auto const maximum = data.maximum() * 1.1f;
  auto const minimum = data.minimum() * 0.9f;
  auto const span = maximum == minimum ? 1.0f : maximum - minimum;
  auto const scale = bounds.height() / span;
  const auto& samples = data.samples();
  auto last_point = gfx::PointF(
      bounds.left, bounds.bottom - (samples.front() - data.minimum()) * scale);
  auto x_step = bounds.width() / samples.size();
  auto sum = 0.0f;
  for (auto const sample : samples) {
    sum += sample;
    auto const curr_point =
        gfx::PointF(last_point.x + x_step,
                    bounds.bottom - (sample - data.minimum()) * scale);
    (*canvas)->DrawLine(last_point, curr_point, brush, 1.0f);
    last_point = curr_point;
  }
  auto const avg = sum / samples.size();
  auto const avg_y = bounds.bottom - (avg - data.minimum()) * scale;
  (*canvas)->DrawLine(gfx::PointF(bounds.left, avg_y),
                      gfx::PointF(bounds.right, avg_y), brush, 2.0f);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// MetricsView::View
//
class MetricsView::View final : public ui::LayerOwner,
                                public ui::LayerOwnerDelegate {
 public:
  explicit View(ui::AnimatableWindow* widget);
  ~View() final = default;

  void DidBeginAnimationFrame(base::Time now);
  void DidCreateParentLayer(ui::Layer* parent_layer);
  void DidRecreateParentLayer(ui::Layer* parent_layer);

  // Returns true if painting is completed, otherwise returns false.
  bool Paint();
  void RecordTime(const base::TimeTicks& now);
  void WillDestroyWidget();

 private:
  friend class TimingScope;

  // ui::LayerOwnerDelegate
  void DidRecreateLayer(ui::Layer* old_layer) final;

  std::unique_ptr<gfx::Canvas> canvas_;
  metrics::Sampling frame_duration_data_;
  metrics::Sampling frame_latency_data_;
  base::TimeTicks last_record_time_;
  std::unique_ptr<gfx::TextFormat> text_format_;
  ui::AnimatableWindow* const widget_;

  DISALLOW_COPY_AND_ASSIGN(View);
};

MetricsView::View::View(ui::AnimatableWindow* widget)
    : frame_duration_data_(kNumberOfSamples),
      frame_latency_data_(kNumberOfSamples),
      last_record_time_(metrics::Sampling::NowTimeTicks()),
      text_format_(new gfx::TextFormat(L"Consolas", 11.5)),
      widget_(widget) {
  set_layer_owner_delegate(this);
}

void MetricsView::View::DidBeginAnimationFrame(base::Time now) {
  TRACE_EVENT_WITH_FLOW0("paint", "MetricsView::DidBeginAnimationFrame", this,
                         TRACE_EVENT_FLAG_FLOW_IN);
  if (Paint())
    return;
  widget_->RequestAnimationFrame();
}

void MetricsView::View::DidCreateParentLayer(ui::Layer* parent_layer) {
  auto const compositor = paint::PaintThread::instance()->compositor();
  SetLayer(new ui::Layer(compositor));
  parent_layer->AppendLayer(layer());
  layer()->SetBounds(widget_->GetContentsBounds());
  // Note: It is too early to call Compositor::CommitIfNeeded(), since
  // main visual tree isn't committed yet.
}

void MetricsView::View::DidRecreateParentLayer(ui::Layer* parent_layer) {
  RecreateLayer();
  parent_layer->AppendLayer(layer());
}

bool MetricsView::View::Paint() {
  if (!canvas_)
    canvas_.reset(layer()->CreateCanvas());
  else if (canvas_->GetLocalBounds() != widget_->GetContentsBounds())
    canvas_->SetBounds(widget_->GetContentsBounds());

  auto const canvas = canvas_.get();
  if (!canvas->IsReady())
    return false;
  auto const bounds = canvas->GetLocalBounds();

  std::basic_ostringstream<base::char16> stream;
  stream << L"Frame latency=" << frame_latency_data_.minimum() << L" "
         << frame_latency_data_.maximum() << L" " << frame_latency_data_.last()
         << std::endl;
  stream << L"Frame duration=" << frame_duration_data_.minimum() << L" "
         << frame_duration_data_.maximum() << L" "
         << frame_duration_data_.last() << std::endl;

  auto const text_layout =
      text_format_->CreateLayout(stream.str(), bounds.size());

  auto const alpha = 0.4f;
  auto const radius = 5.0f;
  gfx::Brush bgcolor(canvas, gfx::ColorF(0, 0, 0, alpha));
  gfx::Brush text_brush(canvas, gfx::ColorF(gfx::ColorF::White, 0.7f));
  gfx::Brush graph_brush1(canvas, gfx::ColorF(gfx::ColorF::Red, alpha));
  gfx::Brush graph_brush2(canvas, gfx::ColorF(gfx::ColorF::Blue, alpha));
  gfx::RectF graph_bounds(bounds.left, bounds.bottom - 50, bounds.right,
                          bounds.bottom);
  gfx::Canvas::DrawingScope drawing_scope(canvas);
  canvas->AddDirtyRect(bounds);
  canvas->Clear(gfx::ColorF());
  (*canvas)->FillRoundedRectangle(D2D1::RoundedRect(bounds, radius, radius),
                                  bgcolor);

  PaintSamples(canvas, graph_brush1, graph_bounds, frame_latency_data_);
  graph_bounds = graph_bounds.Offset(0, -30);
  PaintSamples(canvas, graph_brush2, graph_bounds, frame_duration_data_);

  (*canvas)->DrawTextLayout(gfx::PointF(10, 10), *text_layout, text_brush,
                            D2D1_DRAW_TEXT_OPTIONS_CLIP);
  return true;
}

void MetricsView::View::RecordTime(const base::TimeTicks& now) {
  frame_latency_data_.AddSample(now - last_record_time_);
  last_record_time_ = now;
}

void MetricsView::View::WillDestroyWidget() {
  DestroyLayer();
}

// ui::LayerOwnerDelegate
void MetricsView::View::DidRecreateLayer(ui::Layer* old_layer) {
  canvas_.reset();
}

//////////////////////////////////////////////////////////////////////
//
// MetricsView::TimingScope
//
MetricsView::TimingScope::TimingScope(MetricsView* view)
    : start_(metrics::Sampling::NowTimeTicks()), view_(view) {}

MetricsView::TimingScope::~TimingScope() {
  auto const end = metrics::Sampling::NowTimeTicks();
  view_->view_->frame_duration_data_.AddSample(end - start_);
  view_->RequestAnimationFrame();
}

//////////////////////////////////////////////////////////////////////
//
// MetricsView
//
MetricsView::MetricsView() : view_(new View(this)) {
  SetBounds(gfx::Rect(gfx::Size(kViewWidth, kViewHeight)));
}

MetricsView::~MetricsView() {}

void MetricsView::RecordTime() {
  auto const now = metrics::Sampling::NowTimeTicks();
  paint::PaintThread::instance()->PostTask(
      FROM_HERE, base::Bind(&MetricsView::View::RecordTime,
                            base::Unretained(view_.get()), now));
  RequestAnimationFrame();
}

// ui::AnimationFrameHandler
void MetricsView::DidBeginAnimationFrame(base::Time now) {
  TRACE_EVENT_WITH_FLOW0("paint", "MetricsView::DidBeginAnimationFrame",
                         view_.get(), TRACE_EVENT_FLAG_FLOW_OUT);
  paint::PaintThread::instance()->SchedulePaintTask(
      base::Bind(&MetricsView::View::DidBeginAnimationFrame,
                 base::Unretained(view_.get()), now));
}

// ui::LayerOwnerDelegate
void MetricsView::DidRecreateLayer(ui::Layer* old_layer) {
  layer()->RemoveLayer(view_->layer());
  paint::PaintThread::instance()->PostTask(
      FROM_HERE, base::Bind(&MetricsView::View::DidRecreateParentLayer,
                            base::Unretained(view_.get()), layer()));
}

// ui::Widget
void MetricsView::DidRealize() {
  ui::Widget::DidRealize();
  SetLayer(new ui::Layer());
  set_layer_owner_delegate(this);
  paint::PaintThread::instance()->PostTask(
      FROM_HERE, base::Bind(&MetricsView::View::DidCreateParentLayer,
                            base::Unretained(view_.get()), layer()));
}

void MetricsView::WillDestroyWidget() {
  AnimatableWindow::WillDestroyWidget();
  view_->WillDestroyWidget();
}

}  // namespace views
