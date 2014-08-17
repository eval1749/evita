// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/window_animator.h"

#include "evita/gfx/rect_conversions.h"
#include "evita/ui/animation/animatables.h"
#include "evita/ui/animation/animatable_window.h"
#include "evita/ui/animation/animation_value.h"
#include "evita/ui/animation/animator.h"
#include "evita/ui/compositor/layer.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// Animation
//
class WindowAnimator::Animation : public Animatable, public AnimationObserver {
  private: AnimatableWindow* window_;

  protected: Animation(AnimatableWindow* window);
  public: virtual ~Animation();

  protected: base::TimeDelta animation_duration() const;
  protected: Layer* parent_layer() const;
  protected: AnimatableWindow* window() const { return window_; }

  // AnimationObserver
  private: void DidAnimate(Animatable* animatable) override;

  DISALLOW_COPY_AND_ASSIGN(Animation);
};

WindowAnimator::Animation::Animation(AnimatableWindow* window)
    : window_(window) {
  window_->AddObserver(this);
}

WindowAnimator::Animation::~Animation() {
  window_->RemoveObserver(this);
}

base::TimeDelta WindowAnimator::Animation::animation_duration() const {
  return base::TimeDelta::FromMilliseconds(16 * 7);
}

Layer* WindowAnimator::Animation::parent_layer() const {
  return window_->container_widget().layer();
}

// AnimationObserver
void WindowAnimator::Animation::DidAnimate(Animatable* animatable) {
  // Start window animation when content of |window_| is ready.
  // We don't need to know content change of |window| anymore.
  window_->RemoveObserver(this);
  animatable->animator()->ScheduleAnimation(this);
}

namespace {

//////////////////////////////////////////////////////////////////////
//
// RealizeAnimation
//
class RealizeAnimation final : public WindowAnimator::Animation {
  public: RealizeAnimation(AnimatableWindow* window);
  public: virtual ~RealizeAnimation() = default;

  // Animatable
  private: virtual void Animate(base::Time time) override;

  DISALLOW_COPY_AND_ASSIGN(RealizeAnimation);
};

RealizeAnimation::RealizeAnimation(AnimatableWindow* window)
  : Animation(window) {
}

// Animatable
void RealizeAnimation::Animate(base::Time) {
  parent_layer()->AppendChildLayer(window()->layer());
  FinishAnimation();
}

//////////////////////////////////////////////////////////////////////
//
// ReplaceAnimation
//
class ReplaceAnimation final : public WindowAnimator::Animation {
  private: AnimatableWindow* old_window_;

  public: ReplaceAnimation(AnimatableWindow* new_window,
                           AnimatableWindow* old_window);
  public: virtual ~ReplaceAnimation() = default;

  // Animatable
  private: virtual void Animate(base::Time time) override;

  DISALLOW_COPY_AND_ASSIGN(ReplaceAnimation);
};

ReplaceAnimation::ReplaceAnimation(AnimatableWindow* new_window,
                                   AnimatableWindow* old_window)
  : Animation(new_window), old_window_(old_window) {
}

// Animatable
void ReplaceAnimation::Animate(base::Time) {
  parent_layer()->AppendChildLayer(window()->layer());
  parent_layer()->RemoveChildLayer(old_window_->layer());
  old_window_->DestroyWidget();
  FinishAnimation();
}

//////////////////////////////////////////////////////////////////////
//
// SlideInFromBottomAnimation
//
class SlideInFromBottomAnimation final : public WindowAnimator::Animation {
  private: AnimatableWindow* above_window_;
  private: float above_window_height_end_;
  private: float margin_height_;
  private: std::unique_ptr<AnimationFloat> value_;

  public: SlideInFromBottomAnimation(AnimatableWindow* above_window,
                                   float above_window_end_height,
                                   AnimatableWindow* below_window,
                                   float margin_height);
  public: virtual ~SlideInFromBottomAnimation() = default;

  // Animatable
  private: virtual void Animate(base::Time time) override;

  DISALLOW_COPY_AND_ASSIGN(SlideInFromBottomAnimation);
};

SlideInFromBottomAnimation::SlideInFromBottomAnimation(
    AnimatableWindow* above_window, float above_window_end_height,
    AnimatableWindow* below_window, float margin_height)
  : Animation(below_window), above_window_(above_window),
    above_window_height_end_(above_window_end_height),
    margin_height_(margin_height) {
  DCHECK_LT(above_window_height_end_, above_window->layer()->bounds().height());
  DCHECK_EQ(above_window_height_end_, ::floor(above_window_height_end_));
}

// Animatable
void SlideInFromBottomAnimation::Animate(base::Time now) {
  if (!value_) {
    value_.reset(new AnimationFloat(
        now, animation_duration(),
        above_window_->layer()->bounds().height(), above_window_height_end_));
    parent_layer()->AppendChildLayer(window()->layer());
  }
  auto const above_window_height = value_->Compute(now);
  above_window_->SetBounds(gfx::Rect(
      above_window_->bounds().origin(),
      gfx::Size(above_window_->bounds().width(),
      static_cast<int>(above_window_height))));
  window()->layer()->SetBounds(gfx::RectF(
      above_window_->layer()->bounds().bottom_left() +
          gfx::SizeF(0.0f, margin_height_),
      window()->layer()->bounds().size()));

  if (above_window_height == above_window_height_end_)
    FinishAnimation();
  else
    ui::Animator::instance()->ScheduleAnimation(this);
}

//////////////////////////////////////////////////////////////////////
//
// SlideInFromRightAnimation
//
class SlideInFromRightAnimation final : public WindowAnimator::Animation {
  private: AnimatableWindow* left_window_;
  private: float left_window_width_end_;
  private: float margin_width_;
  private: std::unique_ptr<AnimationFloat> value_;

  public: SlideInFromRightAnimation(AnimatableWindow* left_window,
                                   float left_window_end_width,
                                   AnimatableWindow* right_window,
                                   float margin_width);
  public: virtual ~SlideInFromRightAnimation() = default;

  // Animatable
  private: virtual void Animate(base::Time time) override;

  DISALLOW_COPY_AND_ASSIGN(SlideInFromRightAnimation);
};

SlideInFromRightAnimation::SlideInFromRightAnimation(
    AnimatableWindow* left_window, float left_window_end_width,
    AnimatableWindow* right_window, float margin_width)
  : Animation(right_window), left_window_(left_window),
    left_window_width_end_(left_window_end_width),
    margin_width_(margin_width) {
  DCHECK_LT(left_window_width_end_, left_window->layer()->bounds().width());
  DCHECK_EQ(left_window_width_end_, ::floor(left_window_width_end_));
}

// Animatable
void SlideInFromRightAnimation::Animate(base::Time now) {
  if (!value_) {
    value_.reset(new AnimationFloat(
        now, animation_duration(),
        left_window_->layer()->bounds().width(), left_window_width_end_));
    parent_layer()->AppendChildLayer(window()->layer());
  }
  auto const left_window_width = value_->Compute(now);
  left_window_->SetBounds(gfx::Rect(
      left_window_->bounds().origin(),
      gfx::Size(static_cast<int>(left_window_width),
                left_window_->bounds().height())));
  window()->layer()->SetBounds(gfx::RectF(
      left_window_->layer()->bounds().top_right() +
          gfx::SizeF(margin_width_, 0.0f),
      window()->layer()->bounds().size()));

  if (left_window_width == left_window_width_end_)
    FinishAnimation();
  else
    ui::Animator::instance()->ScheduleAnimation(this);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// WindowAnimator
//
WindowAnimator::WindowAnimator() : animations_(new Animatables()) {
}

WindowAnimator::~WindowAnimator() {
}

void WindowAnimator::Realize(AnimatableWindow* window) {
  RegisterAnimation(new RealizeAnimation(window));
}

void WindowAnimator::RegisterAnimation(Animation* animation){
  animations_->AddAnimatable(animation);
}

void WindowAnimator::Replace(AnimatableWindow* new_window,
                   AnimatableWindow* old_window) {
  RegisterAnimation(new ReplaceAnimation(new_window, old_window));
}

void WindowAnimator::SlideInFromBottom(AnimatableWindow* above_window,
                                       float above_window_height,
                                       AnimatableWindow* below_window,
                                       float margin_height) {
  RegisterAnimation(new SlideInFromBottomAnimation(
      above_window, above_window_height, below_window, margin_height));
}

void WindowAnimator::SlideInFromRight(AnimatableWindow* left_window,
                                      float left_window_height,
                                      AnimatableWindow* right_window,
                                      float margin_width) {
  RegisterAnimation(new SlideInFromRightAnimation(
      left_window, left_window_height, right_window, margin_width));
}

}  // namespace ui
