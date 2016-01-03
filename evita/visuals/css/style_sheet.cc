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
void StyleSheet::RemoveObserver(StyleSheetObserver* observer) const {
  observers_.RemoveObserver(observer);
}

void StyleSheet::RemoveRule(size_t index) {
  auto it = rules_.begin();
  size_t position = 0;
  while (it != rules_.end()) {
    if (position == index) {
      const auto rule = *it;
      rules_.erase(it);
      FOR_EACH_OBSERVER(StyleSheetObserver, observers_, DidAddRule(*rule));
      delete rule;
      return;
    }
    ++position;
  }
  NOTREACHED() << "Bad index " << index << " max=" << rules_.size();
}

}  // namespace css
}  // namespace visuals
