// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/compositor/layer_animation.h"

#include <memory>

#include "base/logging.h"
#include "evita/ui/animation/animation_value.h"
#include "evita/ui/compositor/layer.h"

namespace ui {

namespace {

//////////////////////////////////////////////////////////////////////
//
// SlideInFormLeftAnimation
//
class SlideInFormLeftAnimation : public LayerAnimation {
  private: std::unique_ptr<AnimationFloat> animation_value_;
  private: ui::Layer* const left_layer_;
  private: ui::Layer* const right_layer_;

  public: SlideInFormLeftAnimation(ui::Layer* left_layer,
                                   ui::Layer* right_layer);
  public: virtual ~SlideInFormLeftAnimation();

  // Animatable
  private: virtual void Animate(base::Time now) override;

  DISALLOW_COPY_AND_ASSIGN(SlideInFormLeftAnimation);
};

SlideInFormLeftAnimation::SlideInFormLeftAnimation(ui::Layer* left_layer,
                                                   ui::Layer* right_layer)
    : left_layer_(left_layer), right_layer_(right_layer) {
  DCHECK(left_layer_->parent_layer());
  if (right_layer_) {
    DCHECK_EQ(left_layer_->parent_layer(), right_layer_->parent_layer());
  }
}

SlideInFormLeftAnimation::~SlideInFormLeftAnimation() {
}

void SlideInFormLeftAnimation::Animate(base::Time now) {
  if (!animation_value_) {
    animation_value_.reset(new AnimationFloat(
        now, animation_duration(), -left_layer_->bounds().width(), 0.0f));
    left_layer_->StartAnimation(this);
    if (right_layer_)
      right_layer_->StartAnimation(this);
  }
  auto const left = animation_value_->Compute(now);
  left_layer_->SetOrigin(gfx::PointF(left, left_layer_->bounds().top));
  if (left < 0.0f) {
    ScheduleAnimation();
    return;
  }
  if (right_layer_) {
    right_layer_->parent_layer()->RemoveChildLayer(right_layer_);
    right_layer_->EndAnimation();
  }
  left_layer_->EndAnimation();
  FinishAnimation();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// LayerAnimation
//
LayerAnimation::LayerAnimation() {
}

LayerAnimation::~LayerAnimation() {
}

base::TimeDelta LayerAnimation::animation_duration() const {
  return base::TimeDelta::FromMilliseconds(16 * 7);
}

LayerAnimation* LayerAnimation::CreateSlideInFromLeft(
    ui::Layer* left_layer, ui::Layer* right_layer) {
  return new SlideInFormLeftAnimation(left_layer, right_layer);
}

}  // namespace ui
