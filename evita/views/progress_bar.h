// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_PROGRESS_BAR_H_
#define EVITA_VIEWS_PROGRESS_BAR_H_

#include <memory>

#include "evita/gfx/color_f.h"
#include "evita/ui/animation/animatable_window.h"
#include "evita/ui/compositor/layer_owner_delegate.h"

namespace gfx {
class Canvas;
}

namespace views {

class ProgressBar final : public ui::AnimatableWindow,
                          public ui::LayerOwnerDelegate {
  DECLARE_CASTABLE_CLASS(MetricsView, ui::AnimatableWindow);

 public:
  ProgressBar(const ProgressBar& other) = delete;
  ProgressBar();
  ~ProgressBar() final;

  ProgressBar& operator==(const ProgressBar& other) = delete;

  void SetColors(const gfx::ColorF& color, const gfx::ColorF& bgcolor);
  void SetMinMax(float minium, float maximum);
  void SetProgress(float current);

 private:
  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(const base::TimeTicks& time) final;

  // ui::LayerOwnerDelegate
  void DidRecreateLayer(ui::Layer* old_layer) final;

  // ui::Widget
  void DidChangeBounds() override;
  void DidHide() override;
  void DidShow() override;

  gfx::ColorF bgcolor_;
  std::unique_ptr<gfx::Canvas> canvas_;
  gfx::ColorF color_;
  float current_ = 0.0f;
  float maximum_ = 1.0f;
  float minimum_ = 0.0f;
};

}  // namespace views

#endif  // EVITA_VIEWS_PROGRESS_BAR_H_
