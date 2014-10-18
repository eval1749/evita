// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_animation_frame_handler_h)
#define INCLUDE_evita_ui_animation_animation_frame_handler_h

#include "base/basictypes.h"

namespace base {
class Time;
}

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// AnimationFrameHandler
//
class AnimationFrameHandler {
  protected: AnimationFrameHandler();
  public: virtual ~AnimationFrameHandler();

  // Request animation frame.
  public: void CancelAnimationFrameRequest();

  public: void HandleAnimationFrame(base::Time time);

  // Called when animation frame started at |time|.
  protected: virtual void DidBeginAnimationFrame(base::Time time) = 0;

  // Request animation frame.
  public: void RequestAnimationFrame();

  DISALLOW_COPY_AND_ASSIGN(AnimationFrameHandler);
};

}   // namespace ui

#endif //!defined(INCLUDE_evita_ui_animation_animation_frame_handler_h)
