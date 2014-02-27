// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/style_sheet.h"

#include "base/logging.h"
#include "evita/css/style.h"
#include "evita/css/style_rule.h"
#include "evita/css/style_selector.h"

namespace css {

StyleSheet::StyleSheet() {
}

StyleSheet::~StyleSheet() {
}

void StyleSheet::AddObserver(StyleSheetObserver* observer) const {
  const_cast<StyleSheet*>(this)->observers_.AddObserver(observer);
}

void StyleSheet::AddRule(const base::string16& selector, const Style& style) {
  auto new_style = std::make_unique<Style>(style);
  auto const new_style_ptr = new_style.get();
  selector_map_[selector] = std::move(new_style);
  StyleRule rule(selector, new_style_ptr);
  FOR_EACH_OBSERVER(StyleSheetObserver, observers_, DidAddRule(&rule));
}

const Style* StyleSheet::Find(const base::string16& selector) const {
  auto it = selector_map_.find(selector);
  return it == selector_map_.end() ? nullptr : it->second.get();
}

void StyleSheet::RemoveObserver(StyleSheetObserver* observer) const {
  const_cast<StyleSheet*>(this)->observers_.RemoveObserver(observer);
}

}  // namespace css
