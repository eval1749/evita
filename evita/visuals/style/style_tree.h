// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_STYLE_TREE_H_
#define EVITA_VISUALS_STYLE_STYLE_TREE_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "base/macros.h"
#include "base/observer_list.h"
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
class StyleChangeObserver;

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

  // TODO(eval1749): Do we really need to expose |initial_style()|? As of today,
  // it is used only in tests.
  const css::Style& initial_style() const;

  const Document& document() const { return document_; }
  const css::Media& media() const { return media_; }

  // Monotonically increased style tree version. This version is incremented
  // when |UpdateIfNeeded()| does updating.
  // We exposed |version()| for performance measurement.
  int version() const;

  void AddObserver(StyleChangeObserver* observer) const;
  const css::Style& ComputedStyleOf(const Node& node) const;
  void RemoveObserver(StyleChangeObserver* observer) const;
  void UpdateIfNeeded();

 private:
  class Impl;

  void Clear();
  // css::MediaObserver
  void DidChangeViewportSize() final;
  void DidChangeSystemMetrics() final;

  // css::StyleSheetObserver
  void DidAddRule(const css::Rule& rule) final;
  void DidRemoveRule(const css::Rule& rule) final;

  // DocumentObserver
  void DidAddClass(const Element& element, const base::string16& name);
  void DidChangeInlineStyle(const Element& element,
                            const css::Style* old_style) final;

  const Document& document_;
  const css::Media& media_;
  mutable base::ObserverList<StyleChangeObserver> observers_;
  std::unique_ptr<Impl> impl_;
  const std::vector<css::StyleSheet*> style_sheets_;

  DISALLOW_COPY_AND_ASSIGN(StyleTree);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_STYLE_TREE_H_
