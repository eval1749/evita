// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/metrics_view.h"

#include "evita/gfx/canvas.h"
#include "evita/metrics/sampling.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/compositor/layer_content.h"

namespace views {

namespace {
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
  (*canvas)->Flush();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// MetricsView::Model
//
class MetricsView::Model final {
  friend class TimingScope;

  private: gfx::RectF bounds_;
  private: metrics::Sampling frame_timing_data_;
  private: std::unique_ptr<ui::LayerContent> content_;

  public: Model();
  public: ~Model() = default;

  public: void UpdateView(ui::Layer* layer);

  DISALLOW_COPY_AND_ASSIGN(Model);
};

MetricsView::Model::Model() {
}

void MetricsView::Model::UpdateView(ui::Layer* layer) {
  if (!content_ || content_->layer() != layer)
    content_.reset(new ui::LayerContent(layer));

  auto const bounds = gfx::RectF(layer->bounds().size());

  ui::LayerContent::DrawingScope drawing_scope(content_.get());
  auto const canvas = drawing_scope.canvas();
  (*canvas)->Clear(gfx::ColorF(0, 0, 0, 0.5f));
  PaintSamples(canvas, gfx::Brush(canvas, gfx::ColorF::Red), bounds,
               frame_timing_data_);
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
  view_->model_->frame_timing_data_.AddSample(end - start_);
}

//////////////////////////////////////////////////////////////////////
//
// MetricsView
//
MetricsView::MetricsView() : model_(new Model()) {
}

MetricsView::~MetricsView() {
}

void MetricsView::UpdateView() {
  model_->UpdateView(layer());
}

// ui::Widget
void MetricsView::DidChangeBounds() {
#if 0
  auto const size = gfx::SizeF(150.0f, 50.0f);
  auto const origin = gfx::PointF(
      static_cast<float>(container_widget().bounds().right),
      static_cast<float>(container_widget().bounds().bottom));
  layer()->SetBounds(gfx::RectF(origin - size, size));
#endif
}

void MetricsView::DidRealize() {
  auto const parent_layer = container_widget().layer();
  auto const layer = parent_layer->CreateLayer();
  SetLayer(layer);
  parent_layer->AppendChildLayer(layer);
  layer->SetBounds(gfx::RectF(bounds()));
  DidChangeBounds();
}

}  // namespace views

