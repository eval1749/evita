// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_ANIMATION_ANIMATION_SCHEDULER_H_
#define EVITA_UI_ANIMATION_ANIMATION_SCHEDULER_H_

#include "base/macros.h"

namespace ui {

class AnimationFrameHandler;

//////////////////////////////////////////////////////////////////////
//
// AnimationScheduler
//
class AnimationScheduler {
 public:
  AnimationScheduler();
  virtual ~AnimationScheduler();

  // Request animation frame.
  virtual void CancelAnimationFrameRequest(AnimationFrameHandler* handler);

  // Request animation frame.
  virtual void RequestAnimationFrame(AnimationFrameHandler* handler);

  void SetScheduler(AnimationScheduler* scheduler);

  static AnimationScheduler* GetInstance();

 private:
  AnimationScheduler* impl_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(AnimationScheduler);
};

}  // namespace ui

#endif  // EVITA_UI_ANIMATION_ANIMATION_SCHEDULER_H_
