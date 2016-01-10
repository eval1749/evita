// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_TREE_H_
#define EVITA_VISUALS_LAYOUT_BOX_TREE_H_

#include <iosfwd>
#include <memory>

#include "base/macros.h"
#include "evita/visuals/css/media_observer.h"

namespace visuals {

class Box;
class Document;
class Node;
class RootBox;
class StyleTree;

//////////////////////////////////////////////////////////////////////
//
// BoxTree represents a CSS Box tree for document(node tree) with style tree.
//
class BoxTree final : public css::MediaObserver {
 public:
  BoxTree(const Document& document, const StyleTree& style_tree);
  ~BoxTree();

  RootBox* root_box() const;
  int version() const;

  Box* BoxFor(const Node& node) const;
  void UpdateIfNeeded();

 private:
  class Impl;

  // css::MediaObserver
  void DidChangeViewportSize() final;
  void DidChangeSystemMetrics() final;

  std::unique_ptr<Impl> impl_;

  DISALLOW_COPY_AND_ASSIGN(BoxTree);
};

std::ostream& operator<<(std::ostream& ostream, const BoxTree& box_tree);

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_TREE_H_
