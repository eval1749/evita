// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animation_group.h"

#include "base/logging.h"
#include "base/time/time.h"
#include "common/temporary_change_value.h"
#include "evita/ui/animation/animation_group_member.h"

namespace ui {

AnimationGroup::AnimationGroup() : is_running_(false) {}

AnimationGroup::~AnimationGroup() {}

void AnimationGroup::AddMember(AnimationGroupMember* member) {
  members_.insert(member);
}

bool AnimationGroup::Animate(base::Time time) {
  DCHECK(!is_running_);
  common::TemporaryChangeValue<bool> running_scope(is_running_, true);
  const auto members = members_;
  members_.clear();
  for (auto member : members) {
    if (removed_members_.find(member) != removed_members_.end())
      continue;
    member->Animate(time);
  }
  for (auto removed : removed_members_)
    members_.erase(removed);
  return !members_.empty();
}

void AnimationGroup::RemoveMember(AnimationGroupMember* member) {
  if (is_running_) {
    removed_members_.insert(member);
    return;
  }
  members_.erase(member);
}

}  // namespace ui
