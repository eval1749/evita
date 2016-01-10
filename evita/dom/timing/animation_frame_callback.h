// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TIMING_ANIMATION_FRAME_CALLBACK_H_
#define EVITA_DOM_TIMING_ANIMATION_FRAME_CALLBACK_H_

#include "base/callback.h"
#include "base/location.h"
#include "base/time/time.h"
#include "base/tracking_info.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// AnimationFrameCallback
//
class AnimationFrameCallback final : public base::TrackingInfo {
 public:
  using Callback = base::Callback<void(const base::Time&)>;

  AnimationFrameCallback(const tracked_objects::Location& posted_from,
                         const Callback& callback);
  ~AnimationFrameCallback();

  void Run(const base::Time& time);

 private:
  Callback callback_;
};

}  // namespace dom

#endif  // EVITA_DOM_TIMING_ANIMATION_FRAME_CALLBACK_H_
