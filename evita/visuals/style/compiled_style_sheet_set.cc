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
// CompiledStyleSheetSet::RuleLess
//
// Apply more specific rule. If we can't determine more specific rule,
// we use first appeared rule[1].
// Example:
//  .c1 { color: red; }
//  .c2 { color: blue; }
// When we have above style sheet, color of an element selected by
// "foo.c1.c2" is "red", since ".c1" and ".c2" are same specificity but
// ".c1" is appeared before ".c2".
// [1] https://www.w3.org/TR/css3-selectors/#specificity
bool CompiledStyleSheetSet::RuleLess::operator()(const Rule& result1,
                                                 const Rule& result2) const {
  DVLOG(1) << "RuleLess: " << result1->first << ' ' << result2->first << ' '
           << result1->first.IsMoreSpecific(result2->first) << ' '
           << result2->first.IsMoreSpecific(result1->first);
  const auto more_specific1 = result1->first.IsMoreSpecific(result2->first);
  const auto more_specific2 = result2->first.IsMoreSpecific(result1->first);
  if (more_specific1 != more_specific2)
    return more_specific1;
  return result1->second.position < result2->second.position;
}

//////////////////////////////////////////////////////////////////////
//
// CompiledStyleSheetSet
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

void CompiledStyleSheetSet::ClearCache() {
  cached_matches_.clear();
  rules_.clear();
}

void CompiledStyleSheetSet::CompileStyleSheetsIfNeeded() {
  if (!rules_.empty())
    return;
  for (const auto& style_sheet : style_sheets_) {
    for (const auto& rule : style_sheet->rules())
      CompileRule(*rule);
  }
  if (DLOG_IS_ON(INFO) && VLOG_IS_ON(1)) {
    DVLOG(1) << "Compiled Rules";
    for (const auto& entry : rules_)
      DVLOG(1) << "  " << entry.second.position << ": " << entry.first;
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

// Returns true if this function adds class to |builder|.
static bool AddClass(css::Selector::Builder* builder,
                     const css::Selector& selector,
                     size_t start) {
  const auto& classes = selector.classes();
  DCHECK_LE(start, classes.size());
  if (start == classes.size())
    return false;
  size_t index = 0;
  for (const auto& class_name : classes) {
    if (index >= start)
      builder->AddClass(class_name);
    ++index;
  }
  return true;
}

CompiledStyleSheetSet::MatchSet CompiledStyleSheetSet::Match(
    const css::Selector& selector) const {
  DVLOG(1) << "Match: " << selector;
  MatchSet matched;
  MatchOne(&matched, css::Selector(), selector);
  for (size_t index = 0; index <= selector.classes().size(); ++index) {
    if (!selector.is_universal() && selector.has_id()) {
      css::Selector::Builder builder;
      builder.SetTagName(selector.tag_name()).SetId(selector.id());
      if (index == 0)
        MatchOne(&matched, builder.Build(), selector);
      if (AddClass(&builder, selector, index))
        MatchOne(&matched, builder.Build(), selector);
    }

    if (!selector.is_universal()) {
      css::Selector::Builder builder;
      builder.SetTagName(selector.tag_name());
      if (index == 0)
        MatchOne(&matched, builder.Build(), selector);
      if (AddClass(&builder, selector, index))
        MatchOne(&matched, builder.Build(), selector);
    }

    if (selector.has_id()) {
      css::Selector::Builder builder;
      builder.SetId(selector.id());
      if (index == 0)
        MatchOne(&matched, builder.Build(), selector);
      if (AddClass(&builder, selector, index))
        MatchOne(&matched, builder.Build(), selector);
    }

    css::Selector::Builder builder;
    if (!AddClass(&builder, selector, index))
      continue;
    MatchOne(&matched, builder.Build(), selector);
  }
  return std::move(matched);
}

static bool StartsWith(const css::Selector& selector1,
                       const css::Selector& selector2) {
  if (selector1.tag_name() != selector2.tag_name())
    return false;
  if (selector1.id() != selector2.id())
    return false;
  if (selector1.classes().empty() || selector2.classes().empty())
    return selector1.classes().empty() == selector2.classes().empty();
  if (selector1.classes().size() > selector2.classes().size())
    return false;
  return *selector1.classes().begin() == *selector2.classes().begin();
}

void CompiledStyleSheetSet::MatchOne(MatchSet* match_set,
                                     const css::Selector& needle,
                                     const css::Selector& selector) const {
  DVLOG(1) << "  MatchOne: needle=" << needle;
  auto runner = rules_.lower_bound(needle);
  while (runner != rules_.end()) {
    if (!StartsWith(runner->first, needle)) {
      DVLOG(1) << "    exit " << runner->first << " doesn't start with "
               << needle;
      return;
    }
    if (selector.IsSubsetOf(runner->first)) {
      DVLOG(1) << "    " << runner->first;
      const auto& result = match_set->emplace(runner);
      if (!result.second) {
        DVLOG(1) << "  Matched Rules:";
        for (const auto& rule : *match_set)
          DVLOG(1) << "    " << rule->first;
        NOTREACHED();
      }
    } else {
      DVLOG(1) << "    skip " << runner->first;
    }
    ++runner;
  }
}

void CompiledStyleSheetSet::Merge(css::Style* passed_style,
                                  const css::Selector& selector) const {
  DCHECK(!selector.is_universal()) << selector;
  const_cast<CompiledStyleSheetSet*>(this)->CompileStyleSheetsIfNeeded();

  {
    const auto& present = cached_matches_.find(selector);
    if (present != cached_matches_.end()) {
      css::StyleEditor().Merge(passed_style, *present->second);
      return;
    }
  }

  DVLOG(1) << "CompiledStyleSheetSet::Merge: " << selector;

  const auto& matched = Match(selector);
  auto style = std::make_unique<css::Style>();
  for (const auto& rule : matched)
    css::StyleEditor().Merge(style.get(), *rule->second.style);

  css::StyleEditor().Merge(passed_style, *style);
  const auto& result = cached_matches_.emplace(selector, std::move(style));
  DCHECK(result.second) << "Cached matches should not have entry of "
                        << selector;
}

void CompiledStyleSheetSet::RemoveObserver(
    css::StyleSheetObserver* observer) const {
  observers_.RemoveObserver(observer);
}

// css::StyleSheetObserver
void CompiledStyleSheetSet::DidInsertRule(const css::Rule& new_rule,
                                          size_t index) {
  ClearCache();
  FOR_EACH_OBSERVER(css::StyleSheetObserver, observers_,
                    DidInsertRule(new_rule, index));
}

void CompiledStyleSheetSet::DidRemoveRule(const css::Rule& old_rule,
                                          size_t index) {
  ClearCache();
  FOR_EACH_OBSERVER(css::StyleSheetObserver, observers_,
                    DidRemoveRule(old_rule, index));
}

}  // namespace visuals
