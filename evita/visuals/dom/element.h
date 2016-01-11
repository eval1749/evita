// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_ELEMENT_H_
#define EVITA_VISUALS_DOM_ELEMENT_H_

#include <memory>
#include <vector>

#include "evita/visuals/dom/container_node.h"

#include "base/strings/string16.h"

namespace visuals {

namespace css {
class Style;
}

//////////////////////////////////////////////////////////////////////
//
// Element
//
class Element final : public ContainerNode {
  DECLARE_VISUAL_NODE_FINAL_CLASS(Element, ContainerNode);

 public:
  Element(Document* document,
          const base::StringPiece16& tag_name,
          const base::StringPiece16& id);
  Element(Document* document, const base::StringPiece16& tag_name);
  ~Element() final;

  const std::vector<base::string16>& class_list() const { return class_list_; }
  const css::Style* inline_style() const { return inline_style_.get(); }
  const base::string16& tag_name() const { return node_name(); }

 private:
  std::vector<base::string16> class_list_;
  std::unique_ptr<css::Style> inline_style_;

  DISALLOW_COPY_AND_ASSIGN(Element);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_ELEMENT_H_
