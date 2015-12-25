// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_ANIMATION_ANIMATION_VALUE_H_
#define EVITA_UI_ANIMATION_ANIMATION_VALUE_H_

#include <unordered_set>

#include "base/macros.h"
#include "base/time/time.h"
#include "common/memory/singleton.h"
#include "evita/gfx/point_f.h"
#include "evita/gfx/size_f.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// AnimationFloat
//
class AnimationFloat final {
 public:
  AnimationFloat(base::Time start_time,
                 base::TimeDelta duration,
                 float start_value,
                 float end_value);
  AnimationFloat(base::TimeDelta duration, float start_value, float end_value);
  ~AnimationFloat();

  float end_value() const { return end_value_; }
  bool is_started() const { return !start_time_.is_null(); }

  float Compute(base::Time current_time) const;
  void Start(base::Time start_Time);

 private:
  const float end_value_;
  base::TimeDelta duration_;
  base::Time start_time_;
  const float start_value_;

  DISALLOW_COPY_AND_ASSIGN(AnimationFloat);
};

//////////////////////////////////////////////////////////////////////
//
// AnimationPoint
//
class AnimationPoint final {
 public:
  AnimationPoint(base::Time start_time,
                 base::TimeDelta duration,
                 const gfx::PointF& start_value,
                 const gfx::PointF& end_value);
  ~AnimationPoint();

  gfx::PointF end_value() const { return end_value_; }

  gfx::PointF Compute(base::Time current_time) const;

 private:
  const gfx::PointF end_value_;
  base::TimeDelta duration_;
  base::Time start_time_;
  const gfx::PointF start_value_;

  DISALLOW_COPY_AND_ASSIGN(AnimationPoint);
};

//////////////////////////////////////////////////////////////////////
//
// AnimationSize
//
class AnimationSize final {
 public:
  AnimationSize(base::Time start_time,
                base::TimeDelta duration,
                const gfx::SizeF& start_value,
                const gfx::SizeF& end_value);
  ~AnimationSize();

  gfx::SizeF end_value() const { return end_value_; }

  gfx::SizeF Compute(base::Time current_time) const;

 private:
  const gfx::SizeF end_value_;
  base::TimeDelta duration_;
  base::Time start_time_;
  const gfx::SizeF start_value_;

  DISALLOW_COPY_AND_ASSIGN(AnimationSize);
};

}  // namespace ui

#endif  // EVITA_UI_ANIMATION_ANIMATION_VALUE_H_
