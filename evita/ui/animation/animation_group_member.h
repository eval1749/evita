// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_ANIMATION_ANIMATION_GROUP_MEMBER_H_
#define EVITA_UI_ANIMATION_ANIMATION_GROUP_MEMBER_H_

#include "base/macros.h"

namespace base {
class Time;
}

namespace ui {

class AnimationGroupMember {
 public:
  virtual ~AnimationGroupMember();

  virtual void Animate(base::Time time) = 0;

 protected:
  AnimationGroupMember();

 private:
  DISALLOW_COPY_AND_ASSIGN(AnimationGroupMember);
};

}  // namespace ui

#endif  // EVITA_UI_ANIMATION_ANIMATION_GROUP_MEMBER_H_
