// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_COMPILED_STYLE_SHEET_SET_H_
#define EVITA_VISUALS_STYLE_COMPILED_STYLE_SHEET_SET_H_

#include <map>
#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/observer_list.h"
#include "evita/visuals/css/selector.h"
#include "evita/visuals/css/style_sheet_observer.h"

namespace visuals {

class ElementNode;

namespace css {
class Style;
class StyleSheet;
}

//////////////////////////////////////////////////////////////////////
//
// CompiledStyleSheetSet
//
class CompiledStyleSheetSet final : public css::StyleSheetObserver {
 public:
  explicit CompiledStyleSheetSet(
      const std::vector<css::StyleSheet*>& style_sheets);
  ~CompiledStyleSheetSet();

  void AddObserver(css::StyleSheetObserver* observer) const;
  void Merge(css::Style* style, const css::Selector& selector) const;
  void RemoveObserver(css::StyleSheetObserver* observer) const;

 private:
  struct Entry {
    Entry(size_t passedPosition, std::unique_ptr<css::Style> passedStyle);
    Entry(const Entry& other) = delete;
    Entry(Entry&& other);
    Entry();
    ~Entry();

    size_t position;
    std::unique_ptr<css::Style> style;
  };

  using RuleMap = std::map<css::Selector, Entry>;

  void CompileStyleSheetsIfNeeded();
  void CompileRule(const css::Rule& rule);
  RuleMap::const_iterator FindFirstMatch(const css::Selector& selector) const;

  // css::StyleSheetObserver
  void DidInsertRule(const css::Rule& new_rule, size_t index);
  void DidRemoveRule(const css::Rule& old_rule, size_t index);

  mutable base::ObserverList<css::StyleSheetObserver> observers_;
  RuleMap rules_;
  const std::vector<css::StyleSheet*> style_sheets_;

  DISALLOW_COPY_AND_ASSIGN(CompiledStyleSheetSet);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_COMPILED_STYLE_SHEET_SET_H_
