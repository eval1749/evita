// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_animation_value_h)
#define INCLUDE_evita_ui_animation_animation_value_h

#include "base/basictypes.h"
#include "base/time/time.h"
#include "common/memory/singleton.h"

#include <unordered_set>

namespace ui {

class AnimationFloat final {
  private: float end_value_;
  private: base::TimeDelta duration_;
  private: base::Time start_time_;
  private: float start_value_;

  public: AnimationFloat(base::Time start_time, base::TimeDelta duration,
                         float start_value, float end_value);
  public: ~AnimationFloat();

  public: float Compute(base::Time current_time) const;

  DISALLOW_COPY_AND_ASSIGN(AnimationFloat);
};

}   // namespace ui

#endif //!defined(INCLUDE_evita_ui_animation_animation_value_h)
