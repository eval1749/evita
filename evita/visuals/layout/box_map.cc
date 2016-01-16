// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/box_map.h"

#include "evita/visuals/dom/document.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/root_box.h"

namespace visuals {

BoxMap::BoxMap(const Document& document) : root_box_(new RootBox(document)) {
  map_.emplace(&document, std::unique_ptr<Box>(root_box_));
}

BoxMap::~BoxMap() {
  BoxEditor().RemoveDescendants(root_box_);
}

Box* BoxMap::BoxFor(const Node& node) const {
  const auto& it = map_.find(&node);
  return it == map_.end() ? nullptr : it->second.get();
}

}  // namespace visuals
