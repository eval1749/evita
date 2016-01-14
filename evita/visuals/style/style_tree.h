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

namespace visuals {

namespace css {
class Media;
class Style;
class StyleSheet;
}

class Document;
class Element;
class Node;
class StyleTreeObserver;

//////////////////////////////////////////////////////////////////////
//
// StyleTree represents a cache of CSS computed value, pre-layout value,
// for each element.
//
class StyleTree final : public css::MediaObserver,
                        public css::StyleSheetObserver,
                        public DocumentObserver {
 public:
  explicit StyleTree(const Document& document,
                     const css::Media& media,
                     const std::vector<css::StyleSheet*>& style_sheets);
  ~StyleTree() final;

  const Document& document() const;

  // TODO(eval1749): Do we really need to expose |initial_style()|? As of today,
  // it is used only in tests.
  const css::Style& initial_style() const;

  const css::Media& media() const;

  // Monotonically increased style tree version. This version is incremented
  // when |UpdateIfNeeded()| does updating.
  // We exposed |version()| for performance measurement.
  int version() const;

  void AddObserver(StyleTreeObserver* observer) const;
  const css::Style& ComputedStyleOf(const Node& node) const;
  void RemoveObserver(StyleTreeObserver* observer) const;
  void UpdateIfNeeded();

 private:
  class Impl;

  void Clear();

  // css::MediaObserver
  void DidChangeViewportSize() final;
  void DidChangeSystemMetrics() final;

  // css::StyleSheetObserver
  void DidInsertRule(const css::Rule& rule, size_t index) final;
  void DidRemoveRule(const css::Rule& rule, size_t index) final;

  // DocumentObserver
  void DidAddClass(const Element& element, const base::string16& name);
  void DidAppendChild(const ContainerNode& parent, const Node& child) final;
  void DidChangeInlineStyle(const Element& element,
                            const css::Style* old_style) final;
  void DidInsertBefore(const ContainerNode& parent,
                       const Node& child,
                       const Node& ref_child) final;
  void DidRemoveChild(const ContainerNode& parent, const Node& child) final;
  void DidRemoveClass(const Element& element, const base::string16& name);
  void DidReplaceChild(const ContainerNode& parent,
                       const Node& child,
                       const Node& ref_child) final;

  std::unique_ptr<Impl> impl_;
  const std::vector<css::StyleSheet*> style_sheets_;

  DISALLOW_COPY_AND_ASSIGN(StyleTree);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_STYLE_TREE_H_
