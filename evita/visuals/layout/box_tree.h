// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_TREE_H_
#define EVITA_VISUALS_LAYOUT_BOX_TREE_H_

#include <iosfwd>
#include <memory>

#include "base/macros.h"
#include "evita/visuals/css/media_observer.h"
#include "evita/visuals/dom/document_observer.h"
#include "evita/visuals/view/public/selection_observer.h"
#include "evita/visuals/view/public/selection_model.h"
#include "evita/visuals/style/style_tree_observer.h"

namespace visuals {

class Box;
class BoxSelection;
class ViewLifecycle;
class Node;
class RootBox;
class Selection;
class StyleTree;

//////////////////////////////////////////////////////////////////////
//
// BoxTree represents a CSS Box tree for document(node tree) with style tree.
//
class BoxTree final : public css::MediaObserver,
                      public DocumentObserver,
                      public SelectionObserver,
                      public StyleTreeObserver {
 public:
  BoxTree(ViewLifecycle* lifecycle,
          const Selection& selection,
          const StyleTree& style_tree);
  ~BoxTree();

  RootBox* root_box() const;
  int version() const;

  Box* BoxFor(const Node& node) const;
  void ScheduleForcePaint();
  void UpdateIfNeeded();

 private:
  class Impl;

  BoxSelection ComputeSelection() const;
  void UpdateSelectionIfNeeded();

  // css::MediaObserver
  void DidChangeMediaState() final;
  void DidChangeSystemMetrics() final;
  void DidChangeViewportSize() final;

  // DocumentObserver
  void DidAppendChild(const ContainerNode& parent, const Node& child) final;
  void DidChangeInlineStyle(const ElementNode& element,
                            const css::Style* old_style) final;
  void DidInsertBefore(const ContainerNode& parent,
                       const Node& child,
                       const Node& ref_child) final;
  void DidRemoveChild(const ContainerNode& parent, const Node& child) final;
  void DidReplaceChild(const ContainerNode& parent,
                       const Node& child,
                       const Node& ref_child) final;
  void DidSetTextData(const Text& text,
                      const base::string16& new_data,
                      const base::string16& old_data) final;

  // SelectionObserver
  void DidChangeCaretBlink() final;
  void DidChangeSelection(const SelectionModel& new_model,
                          const SelectionModel& old_model) final;

  // StyleTreeObserver
  void DidChangeComputedStyle(const ElementNode& element,
                              const css::Style& old_style) final;

  bool is_selection_changed_ = false;
  // An implementation of |BoxTree|.
  std::unique_ptr<Impl> impl_;
  const Selection& selection_;

  DISALLOW_COPY_AND_ASSIGN(BoxTree);
};

std::ostream& operator<<(std::ostream& ostream, const BoxTree& box_tree);

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_TREE_H_
