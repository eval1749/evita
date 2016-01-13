// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/style_sheet.h"

#include "evita/visuals/css/rule.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_sheet_observer.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// StyleSheet
//
StyleSheet::StyleSheet() {}

StyleSheet::~StyleSheet() {
  for (const auto& rule : rules_)
    delete rule;
}

void StyleSheet::AddRule(const base::StringPiece16& selector,
                         std::unique_ptr<css::Style> style) {
  const auto rule = new Rule(selector, std::move(style));
  rules_.push_back(rule);
  FOR_EACH_OBSERVER(StyleSheetObserver, observers_, DidAddRule(*rule));
}

void StyleSheet::AddObserver(StyleSheetObserver* observer) const {
  observers_.AddObserver(observer);
}

void StyleSheet::InsertRule(const base::StringPiece16& selector,
                            std::unique_ptr<css::Style> style,
                            size_t index) {
  const auto rule = new Rule(selector, std::move(style));
  rules_.insert(rules_.begin() + index, rule);
  FOR_EACH_OBSERVER(StyleSheetObserver, observers_, DidAddRule(*rule));
}

void StyleSheet::RemoveObserver(StyleSheetObserver* observer) const {
  observers_.RemoveObserver(observer);
}

void StyleSheet::RemoveRule(size_t index) {
  const auto& it = rules_.begin() + index;
  const auto& old_rule = *it;
  rules_.erase(it);
  FOR_EACH_OBSERVER(StyleSheetObserver, observers_, DidRemoveRule(*old_rule));
  delete old_rule;
}

}  // namespace css
}  // namespace visuals
