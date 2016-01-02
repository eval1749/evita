// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/root_box.h"

#include "base/logging.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// RootBox
//
RootBox::RootBox() : ContainerBox(this) {}
RootBox::~RootBox() {}

Box* RootBox::GetBoxById(const base::StringPiece16& id) const {
  const auto& it = id_map_.find(id.as_string());
  if (it == id_map_.end())
    return nullptr;
  // TODO(eval1749): We should return a first box in tree order.
  return it->second;
}

bool RootBox::InLayout() const {
  return lifecycle_.state() == BoxTreeLifecycle::State::InLayout;
}

bool RootBox::InPaint() const {
  return lifecycle_.state() == BoxTreeLifecycle::State::InPaint;
}

bool RootBox::IsLayoutClean() const {
  return lifecycle_.IsAtLeast(BoxTreeLifecycle::State::LayoutClean);
}

bool RootBox::IsPaintClean() const {
  return lifecycle_.IsAtLeast(BoxTreeLifecycle::State::PaintClean);
}

void RootBox::RegisterBoxIdIfNeeded(const Box& box) {
  DCHECK(lifecycle_.AllowsTreeMutaions()) << lifecycle_;
  if (box.id().empty())
    return;
  const auto& result =
      id_map_.insert(std::make_pair(box.id(), const_cast<Box*>(&box)));
  DCHECK(result.second) << "id_map_ already has " << box;
}

void RootBox::UnregisterBoxIdIfNeeded(const Box& box) {
  DCHECK(lifecycle_.AllowsTreeMutaions()) << lifecycle_;
  if (box.id().empty())
    return;
  const auto& start = id_map_.find(box.id());
  DCHECK(start != id_map_.end()) << "id_map_ should have " << box;
  for (auto it = start; it != id_map_.end() && it->first == box.id(); ++it) {
    if (it->second == &box) {
      id_map_.erase(it);
      return;
    }
  }
  DVLOG(ERROR) << "id_map_ should have " << box;
}

}  // namespace visuals
