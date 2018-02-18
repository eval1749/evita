// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/scheduler/animation_frame_callback.h"

#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"

namespace dom {

AnimationFrameCallback::AnimationFrameCallback(
    const base::Location& posted_from,
    Callback callback)
    : callback_(std::move(callback)), posted_from_(posted_from) {}

AnimationFrameCallback::~AnimationFrameCallback() {}

void AnimationFrameCallback::Run(const base::TimeTicks& time) {
  std::move(callback_).Run(time);
}

}  // namespace dom
