// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_STYLE_TREE_H_
#define EVITA_VISUALS_STYLE_STYLE_TREE_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "base/macros.h"
#include "evita/visuals/css/media_observer.h"
#include "evita/visuals/css/style_sheet_observer.h"
#include "evita/visuals/dom/document_observer.h"
#include "evita/visuals/view/public/view_lifecycle_client.h"

namespace visuals {

namespace css {
class Media;
class Style;
class StyleSheet;
}

class Document;
class ElementNode;
class Node;
class Selection;
class StyleTreeObserver;

//////////////////////////////////////////////////////////////////////
//
// StyleTree represents a cache of CSS computed value, pre-layout value,
// for each element.
//
class StyleTree final : public ViewLifecycleClient,
                        public css::MediaObserver,
                        public css::StyleSheetObserver,
                        public DocumentObserver {
 public:
  StyleTree(ViewLifecycle* lifecycle,
            const std::vector<css::StyleSheet*>& style_sheets);
  ~StyleTree() final;

  // TODO(eval1749): Do we really need to expose |initial_style()|? As of today,
  // it is used only in tests.
  const css::Style& initial_style() const;

  // Monotonically increased style tree version. This version is incremented
  // when |UpdateIfNeeded()| does updating.
  // We exposed |version()| for performance measurement.
  int version() const;

  void AddObserver(StyleTreeObserver* observer) const;
  const css::Style& ComputedStyleOf(const Node& node) const;
  const css::Style& ComputedStyleOfSelection(const Selection& selection) const;
  void RemoveObserver(StyleTreeObserver* observer) const;
  void UpdateIfNeeded();

 private:
  class Impl;

  void Clear();
  void MarkDirty(const Node& node);

  // css::MediaObserver
  void DidChangeViewportSize() final;
  void DidChangeSystemMetrics() final;

  // css::StyleSheetObserver
  void DidInsertRule(const css::Rule& rule, size_t index) final;
  void DidRemoveRule(const css::Rule& rule, size_t index) final;

  // DocumentObserver
  void DidAddClass(const ElementNode& element, AtomicString class_name) final;
  void DidAppendChild(const ContainerNode& parent, const Node& child) final;
  void DidChangeInlineStyle(const ElementNode& element,
                            const css::Style* old_style) final;
  void DidInsertBefore(const ContainerNode& parent,
                       const Node& child,
                       const Node& ref_child) final;
  void DidRemoveChild(const ContainerNode& parent, const Node& child) final;
  void DidRemoveClass(const ElementNode& element, AtomicString name) final;
  void DidReplaceChild(const ContainerNode& parent,
                       const Node& child,
                       const Node& ref_child) final;
  void DidSetShapeData(const Shape& shape,
                       const ShapeData& new_data,
                       const ShapeData& old_data) final;
  void DidSetTextData(const Text& text,
                      const base::string16& new_data,
                      const base::string16& old_data) final;

  std::unique_ptr<Impl> impl_;
  const std::vector<css::StyleSheet*> style_sheets_;
  const std::unique_ptr<css::Style> selection_style_;

  DISALLOW_COPY_AND_ASSIGN(StyleTree);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_STYLE_TREE_H_
