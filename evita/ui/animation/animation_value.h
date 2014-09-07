// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_animation_value_h)
#define INCLUDE_evita_ui_animation_animation_value_h

#include "base/basictypes.h"
#include "base/time/time.h"
#include "common/memory/singleton.h"
#include "evita/gfx/point_f.h"
#include "evita/gfx/size_f.h"

#include <unordered_set>

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// AnimationFloat
//
class AnimationFloat final {
  private: const float end_value_;
  private: base::TimeDelta duration_;
  private: base::Time start_time_;
  private: const float start_value_;

  public: AnimationFloat(base::Time start_time, base::TimeDelta duration,
                         float start_value, float end_value);
  public: ~AnimationFloat();

  public: float end_value() const { return end_value_; }

  public: float Compute(base::Time current_time) const;

  DISALLOW_COPY_AND_ASSIGN(AnimationFloat);
};

//////////////////////////////////////////////////////////////////////
//
// AnimationPoint
//
class AnimationPoint {
  private: const gfx::PointF end_value_;
  private: base::TimeDelta duration_;
  private: base::Time start_time_;
  private: const gfx::PointF start_value_;

  public: AnimationPoint(base::Time start_time, base::TimeDelta duration,
                         const gfx::PointF& start_value,
                         const gfx::PointF& end_value);
  public: ~AnimationPoint();

  public: gfx::PointF end_value() const { return end_value_; }

  public: gfx::PointF Compute(base::Time current_time) const;

  DISALLOW_COPY_AND_ASSIGN(AnimationPoint);
};

//////////////////////////////////////////////////////////////////////
//
// AnimationSize
//
class AnimationSize {
  private: const gfx::SizeF end_value_;
  private: base::TimeDelta duration_;
  private: base::Time start_time_;
  private: const gfx::SizeF start_value_;

  public: AnimationSize(base::Time start_time, base::TimeDelta duration,
                         const gfx::SizeF& start_value,
                         const gfx::SizeF& end_value);
  public: ~AnimationSize();

  public: gfx::SizeF end_value() const { return end_value_; }

  public: gfx::SizeF Compute(base::Time current_time) const;

  DISALLOW_COPY_AND_ASSIGN(AnimationSize);
};

}   // namespace ui

#endif //!defined(INCLUDE_evita_ui_animation_animation_value_h)
