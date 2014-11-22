// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_animation_group_member)
#define INCLUDE_evita_ui_animation_animation_group_member

#include "base/macros.h"

namespace base {
class Time;
}

namespace ui {

class AnimationGroupMember {
  protected: AnimationGroupMember();
  public: virtual ~AnimationGroupMember();

  public: virtual void Animate(base::Time time) = 0;

  DISALLOW_COPY_AND_ASSIGN(AnimationGroupMember);
};

}  // namespace ui

#endif // !defined(INCLUDE_evita_ui_animation_animation_group_member)
