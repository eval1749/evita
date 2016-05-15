// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/compiled_style_sheet_set.h"

#include "base/strings/string16.h"
#include "evita/base/strings/atomic_string.h"
#include "evita/visuals/css/rule.h"
#include "evita/visuals/css/selector_builder.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_editor.h"
#include "evita/visuals/css/style_sheet.h"
#include "evita/visuals/dom/element.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// CompiledStyleSheetSet::Entry
//
CompiledStyleSheetSet::Entry::Entry(size_t passedPosition,
                                    std::unique_ptr<css::Style> passedStyle)
    : position(passedPosition), style(std::move(passedStyle)) {}

CompiledStyleSheetSet::Entry::Entry(Entry&& other)
    : position(other.position), style(std::move(other.style)) {}

CompiledStyleSheetSet::Entry::Entry() : position(0) {}
CompiledStyleSheetSet::Entry::~Entry() = default;

//////////////////////////////////////////////////////////////////////
//
// StyleSheet
//
CompiledStyleSheetSet::CompiledStyleSheetSet(
    const std::vector<css::StyleSheet*>& style_sheets)
    : style_sheets_(style_sheets) {
  for (const auto& style_sheet : style_sheets_)
    style_sheet->AddObserver(this);
}

CompiledStyleSheetSet::~CompiledStyleSheetSet() {
  for (const auto& style_sheet : style_sheets_)
    style_sheet->RemoveObserver(this);
}

void CompiledStyleSheetSet::AddObserver(
    css::StyleSheetObserver* observer) const {
  observers_.AddObserver(observer);
}

void CompiledStyleSheetSet::CompileStyleSheetsIfNeeded() {
  if (!rules_.empty())
    return;
  for (const auto& style_sheet : style_sheets_) {
    for (const auto& rule : style_sheet->rules())
      CompileRule(*rule);
  }
}

void CompiledStyleSheetSet::CompileRule(const css::Rule& rule) {
  const auto it = rules_.find(rule.selector());
  if (it != rules_.end()) {
    css::StyleEditor().Merge(it->second.style.get(), rule.style());
    return;
  }
  rules_.emplace(
      rule.selector(),
      std::move(Entry(rules_.size(),
                      std::move(std::make_unique<css::Style>(rule.style())))));
}

CompiledStyleSheetSet::RuleMap::const_iterator
CompiledStyleSheetSet::FindFirstMatch(const css::Selector& selector) const {
  auto runner = rules_.lower_bound(selector);
  while (runner != rules_.end()) {
    if (selector.IsSubsetOf(runner->first))
      return runner;
    ++runner;
  }
  return runner;
}

void CompiledStyleSheetSet::Merge(css::Style* style,
                                  const css::Selector& selector) const {
  DCHECK(selector.is_universal()) << selector;
  const_cast<CompiledStyleSheetSet*>(this)->CompileStyleSheetsIfNeeded();
  auto tag_runner = FindFirstMatch(selector);
  auto any_runner =
      FindFirstMatch(css::Selector::Builder::AsUniversalSelector(selector));
  while (tag_runner != rules_.end() && any_runner != rules_.end()) {
    if (tag_runner == any_runner) {
      any_runner = rules_.end();
      break;
    }
    if (!selector.IsSubsetOf(tag_runner->first))
      break;
    if (!selector.IsSubsetOf(any_runner->first))
      break;
    // Apply more specific rule. If we can't determine more specific rule,
    // we use first appeared rule[1].
    // Example:
    //  .c1 { color: red; }
    //  .c2 { color: blue; }
    // When we have above style sheet, color of an element selected by
    // "foo.c1.c2" is "red", since ".c1" and ".c2" are same specificity but
    // ".c1" is appeared before ".c2".
    // [1] https://www.w3.org/TR/css3-selectors/#specificity
    if (tag_runner->first.IsMoreSpecific(any_runner->first) &&
        (!any_runner->first.IsMoreSpecific(tag_runner->first) ||
         tag_runner->second.position < any_runner->second.position)) {
      css::StyleEditor().Merge(style, *tag_runner->second.style);
      ++tag_runner;
      continue;
    }
    css::StyleEditor().Merge(style, *any_runner->second.style);
    ++any_runner;
  }
  while (tag_runner != rules_.end()) {
    if (!selector.IsSubsetOf(tag_runner->first))
      break;
    css::StyleEditor().Merge(style, *tag_runner->second.style);
    ++tag_runner;
  }
  while (any_runner != rules_.end()) {
    if (!selector.IsSubsetOf(any_runner->first))
      break;
    css::StyleEditor().Merge(style, *any_runner->second.style);
    ++any_runner;
  }
}

void CompiledStyleSheetSet::RemoveObserver(
    css::StyleSheetObserver* observer) const {
  observers_.RemoveObserver(observer);
}

// css::StyleSheetObserver
void CompiledStyleSheetSet::DidInsertRule(const css::Rule& new_rule,
                                          size_t index) {
  rules_.clear();
  FOR_EACH_OBSERVER(css::StyleSheetObserver, observers_,
                    DidInsertRule(new_rule, index));
}

void CompiledStyleSheetSet::DidRemoveRule(const css::Rule& old_rule,
                                          size_t index) {
  rules_.clear();
  FOR_EACH_OBSERVER(css::StyleSheetObserver, observers_,
                    DidRemoveRule(old_rule, index));
}

}  // namespace visuals
