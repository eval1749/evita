// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_ELEMENT_NODE_H_
#define EVITA_VISUALS_DOM_ELEMENT_NODE_H_

#include <vector>

#include "evita/visuals/dom/container_node.h"

namespace visuals {

namespace css {
class Style;
}

//////////////////////////////////////////////////////////////////////
//
// ElementNode
//
class ElementNode : public ContainerNode {
  DECLARE_VISUAL_NODE_ABSTRACT_CLASS(ElementNode, ContainerNode);

 public:
  ~ElementNode() override;

  const std::vector<base::string16>& class_list() const { return class_list_; }
  const css::Style* inline_style() const { return inline_style_.get(); }
  const base::string16& tag_name() const { return node_name(); }

 protected:
  ElementNode(Document* document,
              const base::StringPiece16& tag_name,
              const base::StringPiece16& id);
  ElementNode(Document* document, const base::StringPiece16& tag_name);

 private:
  std::vector<base::string16> class_list_;
  std::unique_ptr<css::Style> inline_style_;

  DISALLOW_COPY_AND_ASSIGN(ElementNode);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_ELEMENT_NODE_H_
