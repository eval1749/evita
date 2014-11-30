// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animation_frame_handler.h"

#include "base/time/time.h"
#include "evita/editor/application.h"
#include "evita/ui/animation/animation_scheduler.h"

namespace ui {

AnimationFrameHandler::AnimationFrameHandler() {
}

AnimationFrameHandler::~AnimationFrameHandler() {
  CancelAnimationFrameRequest();
}

void AnimationFrameHandler::CancelAnimationFrameRequest() {
  editor::Application::instance()->animation_scheduler()->
    CancelAnimationFrameRequest(this);
}

void AnimationFrameHandler::HandleAnimationFrame(base::Time time) {
  DidBeginAnimationFrame(time);
}

void AnimationFrameHandler::RequestAnimationFrame() {
  editor::Application::instance()->animation_scheduler()->
    RequestAnimationFrame(this);
}

}  // namespace ui
