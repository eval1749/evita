// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_ANIMATION_ANIMATION_FRAME_HANDLER_H_
#define EVITA_UI_ANIMATION_ANIMATION_FRAME_HANDLER_H_

#include "base/macros.h"

namespace base {
class Time;
}

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// AnimationFrameHandler
//
class AnimationFrameHandler {
 public:
  virtual ~AnimationFrameHandler();

  // Request animation frame.
  void CancelAnimationFrameRequest();

  // Returns type of animation frame providing this handler.
  virtual const char* GetAnimationFrameType() const = 0;

  void HandleAnimationFrame(base::Time time);

  // Request animation frame.
  void RequestAnimationFrame();

 protected:
  AnimationFrameHandler();
  // Called when animation frame started at |time|.
  virtual void DidBeginAnimationFrame(base::Time time) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(AnimationFrameHandler);
};

}  // namespace ui

#endif  // EVITA_UI_ANIMATION_ANIMATION_FRAME_HANDLER_H_
