// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_MAP_H_
#define EVITA_VISUALS_LAYOUT_BOX_MAP_H_

#include <memory>
#include <unordered_map>

#include "base/macros.h"

namespace visuals {

class Box;
class BoxAssigner;
class DocumentLifecycle;
class Node;
class RootBox;

//////////////////////////////////////////////////////////////////////
//
// BoxMap
//
class BoxMap final {
 public:
  explicit BoxMap(DocumentLifecycle* lifecycle);
  ~BoxMap();

  RootBox* root_box() const { return root_box_; }

  Box* BoxFor(const Node& node) const;

 private:
  friend class BoxAssigner;

  std::unordered_map<const Node*, std::unique_ptr<Box>> map_;
  RootBox* const root_box_;

  DISALLOW_COPY_AND_ASSIGN(BoxMap);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_MAP_H_
