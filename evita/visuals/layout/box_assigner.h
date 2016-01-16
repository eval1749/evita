// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_ASSIGNER_H_
#define EVITA_VISUALS_LAYOUT_BOX_ASSIGNER_H_

#include <memory>

#include "evita/visuals/dom/node_visitor.h"

namespace visuals {

class Box;
class BoxMap;
class ContainerBox;
class RootBox;

namespace css {
class Style;
}

//////////////////////////////////////////////////////////////////////
//
// BoxAssigner
//
class BoxAssigner final : public NodeVisitor {
 public:
  explicit BoxAssigner(BoxMap* box_map);
  ~BoxAssigner() final;

  RootBox* root_box() const;

  void Assign(const Node& node, const css::Style& style);

 private:
  Box* BoxFor(const Node& node) const;
  void DetachChildren(ContainerBox* container_box);
  void RegisterBoxFor(const Node& node, std::unique_ptr<Box> box);

// NodeVisitor
#define V(name) void Visit##name(name* node) final;
  FOR_EACH_VISUAL_NODE(V)
#undef V

  BoxMap* const box_map_;
  const css::Style* style_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(BoxAssigner);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_ASSIGNER_H_
