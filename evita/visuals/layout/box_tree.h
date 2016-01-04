// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_TREE_H_
#define EVITA_VISUALS_LAYOUT_BOX_TREE_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "base/macros.h"
#include "evita/visuals/dom/document_observer.h"
#include "evita/visuals/style/style_tree_observer.h"

namespace visuals {

class Box;
class Document;
class RootBox;
class StyleTree;

//////////////////////////////////////////////////////////////////////
//
// BoxTree represents a CSS Box tree for document(node tree) with style tree.
//
class BoxTree final : public DocumentObserver, public StyleTreeObserver {
 public:
  BoxTree(const Document& document, const StyleTree& style_tree);
  ~BoxTree();

  RootBox* root_box() const;

  RootBox* Build();
  Box* BoxFor(const Node& node) const;

 private:
  void Clear();

  // DocumentObserver
  void DidAppendChild(const ContainerNode& parent, const Node& child) final;
  void DidChangeInlineStyle(const Element& element,
                            const css::Style* old_style) final;
  void DidInsertBefore(const ContainerNode& parent,
                       const Node& child,
                       const Node& ref_child) final;
  void WillRemoveChild(const ContainerNode& parent, const Node& child) final;

  // StyleTreeObserver
  void DidClearStyleCache() final;
  void DidRemoveStyleCache(const Element& element,
                           const css::Style& old_style) final;

  std::unordered_map<const Node*, Box*> box_map_;
  const Document& document_;
  std::unique_ptr<RootBox> root_box_;
  const StyleTree& style_tree_;

  DISALLOW_COPY_AND_ASSIGN(BoxTree);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_TREE_H_
