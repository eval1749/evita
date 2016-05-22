// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_STYLE_STYLE_TREE_H_
#define EVITA_TEXT_STYLE_STYLE_TREE_H_

#include <map>
#include <memory>
#include <vector>

#include "evita/css/style_sheet_observer.h"

namespace css {
class Selector;
class Style;
class StyleSheet;
}

namespace visuals {
class CompiledStyleSheetSet;
}

namespace layout {

class ComputedStyle;

//////////////////////////////////////////////////////////////////////
//
// StyleTree
//
class StyleTree final : public css::StyleSheetObserver {
  using CompiledStyleSheetSet = visuals::CompiledStyleSheetSet;

 public:
  explicit StyleTree(const std::vector<css::StyleSheet*> style_sheets);
  StyleTree(const StyleTree& other) = delete;
  ~StyleTree();

  StyleTree& operator=(const StyleTree& other) = delete;

  const ComputedStyle& ComputedStyleOf(const css::Selector& selector) const;

  void AddStyleSheet(const css::StyleSheet& style_sheet);
  void RemoveStyleSheet(const css::StyleSheet& style_sheet);
  void SetZoom(float zoom);

 private:
  void ResetCache();

  // css::StyleSheetObserver
  void DidInsertRule(const css::Rule& new_rule, size_t index);
  void DidRemoveRule(const css::Rule& old_rule, size_t index);

  mutable std::map<css::Selector, std::unique_ptr<ComputedStyle>> style_cache_;
  std::unique_ptr<CompiledStyleSheetSet> style_sheet_set_;
  float zoom_ = 1.0f;
};

}  // namespace layout

#endif  // EVITA_TEXT_STYLE_STYLE_TREE_H_
