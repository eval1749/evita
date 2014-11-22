// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_animation_group)
#define INCLUDE_evita_ui_animation_animation_group

#include <unordered_set>

#include "base/macros.h"

namespace base {
class Time;
}

namespace ui {

class AnimationGroupMember;

class AnimationGroup {
  private: std::unordered_set<AnimationGroupMember*> members_;
  private: std::unordered_set<AnimationGroupMember*> removed_members_;
  private: bool is_running_;

  public: AnimationGroup();
  public: ~AnimationGroup();

  public: void AddMember(AnimationGroupMember* member);
  public: bool Animate(base::Time time);
  public: void RemoveMember(AnimationGroupMember* member);

  DISALLOW_COPY_AND_ASSIGN(AnimationGroup);
};

}  // namespace ui

#endif // !defined(INCLUDE_evita_ui_animation_animation_group)
