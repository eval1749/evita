// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_animatable_h)
#define INCLUDE_evita_ui_animation_animatable_h

#include "base/time/time.h"

namespace base {
class Time;
}

namespace ui {

class Animatable {
  protected: Animatable();
  public: virtual ~Animatable();

  public: virtual void Animate(base::Time time) = 0;

  DISALLOW_COPY_AND_ASSIGN(Animatable);
};

}   // namespace ui

#endif //!defined(INCLUDE_evita_ui_animation_animatable_h)
