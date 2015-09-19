// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animation_value.h"

#include <algorithm>

#include "base/logging.h"

namespace ui {

namespace {
float ComputeRoundValue(float start, float end, float factor) {
  auto const value = ::round(start + (end - start) * factor);
  return start < end ? std::min(value, end) : std::max(value, end);
}

float ComputeValue(float start, float end, float factor) {
  auto const value = start + (end - start) * factor;
  return start < end ? std::min(value, end) : std::max(value, end);
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// AnimationFloat
//
AnimationFloat::AnimationFloat(base::Time start_time,
                               base::TimeDelta duration,
                               float start_value,
                               float end_value)
    : duration_(duration),
      end_value_(end_value),
      start_time_(start_time),
      start_value_(start_value) {}

AnimationFloat::AnimationFloat(base::TimeDelta duration,
                               float start_value,
                               float end_value)
    : AnimationFloat(base::Time(), duration, start_value, end_value) {}

AnimationFloat::~AnimationFloat() {}

float AnimationFloat::Compute(base::Time current_time) const {
  DCHECK(!start_time_.is_null());
  auto const delta = (current_time - start_time_).InMillisecondsF();
  auto const factor =
      static_cast<float>(std::min(delta / duration_.InMillisecondsF(), 1.0));
  DCHECK_GE(factor, 0.0f);
  return ComputeValue(start_value_, end_value_, factor);
}

void AnimationFloat::Start(base::Time start_time) {
  DCHECK(start_time_.is_null());
  start_time_ = start_time;
}

//////////////////////////////////////////////////////////////////////
//
// AnimationPoint
//
AnimationPoint::AnimationPoint(base::Time start_time,
                               base::TimeDelta duration,
                               const gfx::PointF& start_value,
                               const gfx::PointF& end_value)
    : duration_(duration),
      end_value_(end_value),
      start_time_(start_time),
      start_value_(start_value) {}

AnimationPoint::~AnimationPoint() {}

gfx::PointF AnimationPoint::Compute(base::Time current_time) const {
  auto const delta = (current_time - start_time_).InMillisecondsF();
  auto const factor =
      static_cast<float>(std::min(delta / duration_.InMillisecondsF(), 1.0));
  DCHECK_GE(factor, 0.0f);
  return gfx::PointF(ComputeRoundValue(start_value_.x, end_value_.x, factor),
                     ComputeRoundValue(start_value_.y, end_value_.y, factor));
}

//////////////////////////////////////////////////////////////////////
//
// AnimationSize
//
AnimationSize::AnimationSize(base::Time start_time,
                             base::TimeDelta duration,
                             const gfx::SizeF& start_value,
                             const gfx::SizeF& end_value)
    : duration_(duration),
      end_value_(end_value),
      start_time_(start_time),
      start_value_(start_value) {}

AnimationSize::~AnimationSize() {}

gfx::SizeF AnimationSize::Compute(base::Time current_time) const {
  auto const delta = (current_time - start_time_).InMillisecondsF();
  auto const factor =
      static_cast<float>(std::min(delta / duration_.InMillisecondsF(), 1.0));
  DCHECK_GE(factor, 0.0f);
  return gfx::SizeF(
      ComputeRoundValue(start_value_.width, end_value_.width, factor),
      ComputeRoundValue(start_value_.height, end_value_.height, factor));
}

}  // namespace ui
