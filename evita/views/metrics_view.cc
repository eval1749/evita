// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/metrics_view.h"

#include <sstream>

#include "evita/gfx/canvas.h"
#include "evita/gfx/text_format.h"
#include "evita/gfx/text_layout.h"
#include "evita/metrics/sampling.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/compositor/layer_owner_delegate.h"

namespace views {

namespace {

const size_t kNumberOfSamples = 200;
const int kViewHeight = 100;
const int kViewWidth = 250;

void PaintSamples(gfx::Canvas* canvas, const gfx::Brush& brush,
                  const gfx::RectF& bounds, const metrics::Sampling& data) {
  auto const maximum = data.maximum() * 1.1f;
  auto const minimum = data.minimum() * 0.9f;
  auto const span = maximum == minimum ? 1.0f : maximum - minimum;
  auto const scale = bounds.height() / span;
  const auto& samples = data.samples();
  auto last_point = gfx::PointF(
      bounds.left,
      bounds.bottom - (samples.front() - data.minimum()) * scale);
  auto x_step = bounds.width() / samples.size();
  auto sum = 0.0f;
  for (auto const sample : samples) {
    sum += sample;
    auto const curr_point = gfx::PointF(
        last_point.x + x_step,
        bounds.bottom - (sample - data.minimum()) * scale);
    (*canvas)->DrawLine(last_point, curr_point, brush, 1.0f);
    last_point = curr_point;
  }
  auto const avg = sum / samples.size();
  auto const avg_y = bounds.bottom - (avg - data.minimum()) * scale;
  (*canvas)->DrawLine(gfx::PointF(bounds.left, avg_y),
                      gfx::PointF(bounds.right, avg_y),
                      brush, 2.0f);
  canvas->Flush();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// MetricsView::View
//
class MetricsView::View final : public ui::LayerOwnerDelegate {
  friend class TimingScope;

  private: std::unique_ptr<gfx::Canvas> canvas_;
  private: metrics::Sampling frame_duration_data_;
  private: metrics::Sampling frame_latency_data_;
  private: base::TimeTicks last_record_time_;
  private: std::unique_ptr<gfx::TextFormat> text_format_;
  private: ui::Widget* const widget_;

  public: View(ui::Widget* widget);
  public: virtual ~View() = default;

  public: void Animate(base::Time now);
  public: void RecordTime();

  // ui::LayerOwnerDelegate
  private: virtual void DidRecreateLayer(ui::Layer* old_layer) override;

  DISALLOW_COPY_AND_ASSIGN(View);
};

MetricsView::View::View(ui::Widget* widget)
    : frame_duration_data_(kNumberOfSamples),
      frame_latency_data_(kNumberOfSamples),
      last_record_time_(metrics::Sampling::NowTimeTicks()),
      text_format_(new gfx::TextFormat(L"Consolas", 11.5)), widget_(widget) {
}

void MetricsView::View::Animate(base::Time) {
  if (!canvas_)
    canvas_.reset(widget_->layer()->CreateCanvas());
  else if (canvas_->bounds() != widget_->GetContentsBounds())
    canvas_->SetBounds(widget_->GetContentsBounds());

  auto const canvas = canvas_.get();
  auto const bounds = canvas->bounds();

  std::basic_ostringstream<base::char16> stream;
  stream << L"Frame latency=" << frame_latency_data_.minimum() << L" " <<
    frame_latency_data_.maximum() << L" " << frame_latency_data_.last() <<
    std::endl;
  stream << L"Frame duration=" << frame_duration_data_.minimum() << L" " <<
    frame_duration_data_.maximum() << L" " << frame_duration_data_.last() <<
    std::endl;

  auto const text_layout = text_format_->CreateLayout(stream.str(),
                                                      bounds.size());

  auto const alpha = 0.4f;
  auto const radius = 5.0f;
  gfx::Brush bgcolor(canvas, gfx::ColorF(0, 0, 0, alpha));
  gfx::Brush text_brush(canvas, gfx::ColorF(gfx::ColorF::White, alpha));
  gfx::Brush graph_brush1(canvas, gfx::ColorF(gfx::ColorF::Red, alpha));
  gfx::Brush graph_brush2(canvas, gfx::ColorF(gfx::ColorF::Blue, alpha));
  gfx::RectF graph_bounds(bounds.left, bounds.bottom - 50,
                          bounds.right, bounds.bottom);
  gfx::Canvas::DrawingScope drawing_scope(canvas);
  canvas->AddDirtyRect(bounds);
  canvas->Clear(gfx::ColorF());
  (*canvas)->FillRoundedRectangle(
      D2D1::RoundedRect(bounds, radius, radius), bgcolor);

  PaintSamples(canvas, graph_brush1, graph_bounds, frame_latency_data_);
  graph_bounds = graph_bounds.Offset(0, -30);
  PaintSamples(canvas, graph_brush2, graph_bounds, frame_duration_data_);

  (*canvas)->DrawTextLayout(gfx::PointF(10, 10), *text_layout, text_brush,
                                   D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

void MetricsView::View::RecordTime() {
  auto const now = metrics::Sampling::NowTimeTicks();
  frame_latency_data_.AddSample(now - last_record_time_);
  last_record_time_ = now;
}

// ui::LayerOwnerDelegate
void MetricsView::View::DidRecreateLayer(ui::Layer*) {
  canvas_.reset();
}

//////////////////////////////////////////////////////////////////////
//
// MetricsView::TimingScope
//
MetricsView::TimingScope::TimingScope(MetricsView* view)
    : start_(metrics::Sampling::NowTimeTicks()), view_(view) {
}

MetricsView::TimingScope::~TimingScope() {
  auto const end = metrics::Sampling::NowTimeTicks();
  view_->view_->frame_duration_data_.AddSample(end - start_);
}

//////////////////////////////////////////////////////////////////////
//
// MetricsView
//
MetricsView::MetricsView() : view_(new View(this)) {
  SetBounds(gfx::Rect(gfx::Size(kViewWidth, kViewHeight)));
}

MetricsView::~MetricsView() {
}

void MetricsView::Animate(base::Time now) {
  view_->Animate(now);
}

void MetricsView::RecordTime() {
  view_->RecordTime();
}

// ui::Widget
void MetricsView::DidRealize() {
  ui::Widget::DidRealize();
  SetLayer(new ui::Layer());
  set_layer_owner_delegate(view_.get());
}

}  // namespace views
