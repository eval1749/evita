// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_ANIMATION_ANIMATION_GROUP_H_
#define EVITA_UI_ANIMATION_ANIMATION_GROUP_H_

#include <unordered_set>

#include "base/macros.h"

namespace base {
class Time;
}

namespace ui {

class AnimationGroupMember;

class AnimationGroup {
 public:
  AnimationGroup();
  ~AnimationGroup();

  void AddMember(AnimationGroupMember* member);
  bool Animate(base::Time time);
  void RemoveMember(AnimationGroupMember* member);

 private:
  std::unordered_set<AnimationGroupMember*> members_;
  std::unordered_set<AnimationGroupMember*> removed_members_;
  bool is_running_;

  DISALLOW_COPY_AND_ASSIGN(AnimationGroup);
};

}  // namespace ui

#endif  // EVITA_UI_ANIMATION_ANIMATION_GROUP_H_
