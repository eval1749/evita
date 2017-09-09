// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/scheduler/animation_frame_callback.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/tracked_objects.h"

namespace dom {

AnimationFrameCallback::AnimationFrameCallback(
    const tracked_objects::Location& posted_from,
    const Callback& callback)
    : callback_(callback), posted_from_(posted_from) {}

AnimationFrameCallback::~AnimationFrameCallback() {}

void AnimationFrameCallback::Run(const base::TimeTicks& time) {
  callback_.Run(time);
}

}  // namespace dom
