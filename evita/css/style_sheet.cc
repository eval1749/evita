// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/style_sheet.h"

#include "evita/css/rule.h"
#include "evita/css/style.h"
#include "evita/css/style_sheet_observer.h"

namespace css {

//////////////////////////////////////////////////////////////////////
//
// StyleSheet
//
StyleSheet::StyleSheet() {}

StyleSheet::~StyleSheet() {
  for (auto* const rule : rules_)
    delete rule;
}

void StyleSheet::AppendRule(const Selector& selector,
                            std::unique_ptr<css::Style> style) {
  auto* const rule = new Rule(selector, std::move(style));
  const auto index = rules_.size();
  rules_.push_back(rule);
  for (auto& observer : observers_)
    observer.DidInsertRule(*rule, index);
}

void StyleSheet::AddObserver(StyleSheetObserver* observer) const {
  observers_.AddObserver(observer);
}

void StyleSheet::InsertRule(const Selector& selector,
                            std::unique_ptr<css::Style> style,
                            size_t index) {
  auto* const rule = new Rule(selector, std::move(style));
  rules_.insert(rules_.begin() + index, rule);
  for (auto& observer : observers_)
    observer.DidInsertRule(*rule, index);
}

void StyleSheet::RemoveObserver(StyleSheetObserver* observer) const {
  observers_.RemoveObserver(observer);
}

void StyleSheet::RemoveRule(size_t index) {
  const auto& it = rules_.begin() + index;
  auto* const old_rule = *it;
  rules_.erase(it);
  for (auto& observer : observers_)
    observer.DidRemoveRule(*old_rule, index);
  delete old_rule;
}

}  // namespace css
