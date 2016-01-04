// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_TREE_BUILDER_H_
#define EVITA_VISUALS_LAYOUT_BOX_TREE_BUILDER_H_

#include <memory>
#include <unordered_map>

#include "base/macros.h"
#include "evita/visuals/dom/document_observer.h"
#include "evita/visuals/style/style_change_observer.h"

namespace visuals {

namespace css {
class Media;
}

class Box;
class Document;
class RootBox;
class StyleResolver;

//////////////////////////////////////////////////////////////////////
//
// BoxTreeBuilder generates a CSS Box tree from a Document.
//
class BoxTreeBuilder final : public DocumentObserver,
                             public StyleChangeObserver {
 public:
  BoxTreeBuilder(const Document& document, const css::Media& media);
  ~BoxTreeBuilder();

  const RootBox& Build();

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

  // StyleChangeObserver
  void DidClearStyleCache() final;
  void DidRemoveStyleCache(const Element& element,
                           const css::Style& old_style) final;

  std::unordered_map<const Node*, Box*> box_map_;
  const Document& document_;
  std::unique_ptr<RootBox> root_box_;
  std::unique_ptr<StyleResolver> style_resolver_;

  DISALLOW_COPY_AND_ASSIGN(BoxTreeBuilder);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_TREE_BUILDER_H_
