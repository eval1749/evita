// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/window_animator.h"

#include "evita/gfx/rect_conversions.h"
#include "evita/ui/animation/animatable_window.h"
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

  protected: AnimatableWindow* window() const { return window_; }
  protected: Layer* parent_layer() const;

  // AnimationObserver
  private: void DidAnimate(Animatable* animatable) override;
};

WindowAnimator::Animation::Animation(AnimatableWindow* window)
    : window_(window) {
  window_->AddObserver(this);
}

WindowAnimator::Animation::~Animation() {
  window_->RemoveObserver(this);
}

Layer* WindowAnimator::Animation::parent_layer() const {
  return window_->container_widget().layer();
}

// AnimationObserver
void WindowAnimator::Animation::DidAnimate(Animatable*) {
  auto const now = base::Time::Now();
  PlayAnimation(now);
  FinishAnimation();
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
};

RealizeAnimation::RealizeAnimation(AnimatableWindow* window)
  : Animation(window) {
}

// Animatable
void RealizeAnimation::Animate(base::Time) {
  parent_layer()->AppendChildLayer(window()->layer());
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
}

//////////////////////////////////////////////////////////////////////
//
// SplitAnimation
//
class SplitAnimation final : public WindowAnimator::Animation {
  private: AnimatableWindow* old_window_;
  private: gfx::RectF old_new_bounds_;

  public: SplitAnimation(AnimatableWindow* new_window,
                         AnimatableWindow* old_window,
                         const gfx::RectF& old_new_bounds);
  public: virtual ~SplitAnimation() = default;

  // Animatable
  private: virtual void Animate(base::Time time) override;
};

SplitAnimation::SplitAnimation(AnimatableWindow* new_window,
                               AnimatableWindow* old_window,
                               const gfx::RectF& old_new_bounds)
  : Animation(new_window), old_window_(old_window),
    old_new_bounds_(old_new_bounds) {
}

// Animatable
void SplitAnimation::Animate(base::Time) {
  parent_layer()->AppendChildLayer(window()->layer());
  old_window_->SetBounds(gfx::ToEnclosingRect(old_new_bounds_));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// WindowAnimator
//
WindowAnimator::WindowAnimator() {
}

WindowAnimator::~WindowAnimator() {
  while (!animations_.empty()) {
    auto const animation = *animations_.begin();
    animations_.erase(animations_.begin());
    delete animation;
  }
}

void WindowAnimator::Realize(AnimatableWindow* window) {
  RegisterAnimation(new RealizeAnimation(window));
}

void WindowAnimator::RegisterAnimation(Animation* animation){
  animations_.insert(animation);
  animation->AddObserver(this);
}

void WindowAnimator::Replace(AnimatableWindow* new_window,
                   AnimatableWindow* old_window) {
  RegisterAnimation(new ReplaceAnimation(new_window, old_window));
}

void WindowAnimator::Split(AnimatableWindow* new_window,
                           AnimatableWindow* ref_window,
                           const gfx::RectF& ref_bounds) {
  RegisterAnimation(new SplitAnimation(new_window, ref_window, ref_bounds));
}

// AnimationObserver
void WindowAnimator::DidFinishAnimation(Animatable* animatable) {
  auto const present = animations_.find(animatable);
  DCHECK(present != animations_.end());
  auto const animation = *present;
  animations_.erase(present);
  delete animation;
}

}  // namespace ui
