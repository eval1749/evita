// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/progress_bar.h"

#include "evita/gfx/brush.h"
#include "evita/gfx/canvas.h"
#include "evita/ui/compositor/layer.h"

namespace views {

ProgressBar::ProgressBar() : color_(gfx::ColorF(0.0f, 1.0f, 0.0f)) {}
ProgressBar::~ProgressBar() = default;

void ProgressBar::SetColors(const gfx::ColorF& color,
                            const gfx::ColorF& bgcolor) {
  color_ = color;
  bgcolor_ = bgcolor_;
  RequestAnimationFrame();
}

void ProgressBar::SetMinMax(float minium, float maximum) {
  DCHECK_LT(minium, maximum);
  maximum_ = maximum;
  minimum_ = minium;
  current_ = minimum_;
  RequestAnimationFrame();
}

void ProgressBar::SetProgress(float current) {
  DCHECK_GE(current, minimum_);
  DCHECK_LE(current, maximum_);
  current_ = current;
  RequestAnimationFrame();
}

// ui::AnimationFrameHandler
void ProgressBar::DidBeginAnimationFrame(const base::TimeTicks& time) {
  if (!canvas_)
    return;
  canvas_->Clear(bgcolor_);
  const auto& bounds = GetContentsBounds();
  const auto width =
      (current_ - minimum_) * bounds.width() / (maximum_ - minimum_);
  gfx::RectF rect(gfx::PointF(), gfx::SizeF(width, bounds.height()));
  canvas_->FillRectangle(gfx::Brush(canvas_.get(), color_), rect);
}

// ui::LayerOwnerDelegate
void ProgressBar::DidRecreateLayer(ui::Layer*) {
  if (!canvas_)
    return;
  canvas_.reset(layer()->CreateCanvas());
}

// ui::Widget
void ProgressBar::DidChangeBounds() {
  AnimatableWindow::DidChangeBounds();
  if (!canvas_)
    return;
  canvas_->SetBounds(GetContentsBounds());
}

void ProgressBar::DidHide() {
  AnimatableWindow::DidHide();
  canvas_.reset();
}

void ProgressBar::DidShow() {
  AnimatableWindow::DidShow();
  DCHECK(!canvas_);
  if (bounds().empty())
    return;
  canvas_.reset(layer()->CreateCanvas());
}

}  // namespace views
