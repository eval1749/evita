// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_window_animator_h)
#define INCLUDE_evita_ui_animation_window_animator_h

#include <memory>

#include "evita/ui/animation/animation_observer.h"

#include "evita/gfx/rect_f.h"

namespace ui {

class Animatables;
class AnimatableWindow;

class WindowAnimator final {
  public: class Animation;

  private: const std::unique_ptr<Animatables> animations_;

  public: WindowAnimator();
  public: ~WindowAnimator();

  public: void Realize(AnimatableWindow* window);
  private: void RegisterAnimation(Animation* animation);
  public: void Replace(AnimatableWindow* new_window,
                       AnimatableWindow* old_window);
  public: void SlideInFromBottom(AnimatableWindow* above_window,
                                 float above_window_height,
                                 AnimatableWindow* below_window,
                                 float margin_size);
  public: void SlideInFromRight(AnimatableWindow* left_window,
                                float left_window_width,
                                AnimatableWindow* right_window,
                                float margin_width);

  DISALLOW_COPY_AND_ASSIGN(WindowAnimator);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_animation_window_animator_h)
