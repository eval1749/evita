// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_window_animator_h)
#define INCLUDE_evita_ui_animation_window_animator_h

#include "evita/ui/animation/animation_observer.h"

#include <unordered_set>

#include "evita/gfx/rect_f.h"

namespace ui {

class AnimatableWindow;

class WindowAnimator final : public AnimationObserver {
  public: class Animation;

  private: std::unordered_set<Animatable*> animations_;

  public: WindowAnimator();
  public: ~WindowAnimator();

  public: void Realize(AnimatableWindow* window);
  private: void RegisterAnimation(Animation* animation);
  public: void Replace(AnimatableWindow* new_window,
                       AnimatableWindow* old_window);
  public: void SplitHorizontally(AnimatableWindow* left_window,
                                 float left_window_width,
                                 AnimatableWindow* right_window,
                                 float margin_width);
  public: void SplitVertically(AnimatableWindow* above_window,
                               float above_window_height,
                               AnimatableWindow* below_window,
                               float margin_size);

  // AnimationObserver
  private: virtual void DidFinishAnimation(Animatable* animatable) override;

  DISALLOW_COPY_AND_ASSIGN(WindowAnimator);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_animation_window_animator_h)
