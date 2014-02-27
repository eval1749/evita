// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/style_sheet.h"

#include "base/logging.h"
#include "evita/css/style_selector.h"

namespace css {

// TODO(yosi) We should not have |using css::Color|.
using css::Color;

StyleSheet::StyleSheet()
    : base_style_sheet_(DefaultStyleSheet()) {
}

StyleSheet::StyleSheet(bool) : base_style_sheet_(nullptr) {
}

StyleSheet::~StyleSheet() {
}

void StyleSheet::AddRule(const base::string16& selector, const Style& style) {
  selector_map_[selector] = std::make_unique<Style>(style);
  if (selector == StyleSelector::defaults()) {
    ClearCache();
  } else {
    auto const cache = style_cache_.find(selector);
    if (cache != style_cache_.end())
      style_cache_.erase(cache);
  }
}

void StyleSheet::ClearCache() {
  style_cache_.clear();
}

StyleSheet* StyleSheet::DefaultStyleSheet() {
  static StyleSheet* default_style_sheet;
  if (default_style_sheet)
    return default_style_sheet;
  DEFINE_STATIC_LOCAL(StyleSheet, style_sheet, (true));
  Style default_style(Color(0, 0, 0), Color(255, 255, 255));
  default_style.set_font_family(L"Consolas, MS Gothic");
  default_style.set_font_size(10);
  default_style.set_font_style(css::FontStyle::Normal);
  default_style.set_font_weight(css::FontWeight::Normal);
  // TODO(yosi) We should move Style::syntax() to another place.
  default_style.set_syntax(0);
  default_style.set_text_decoration(css::TextDecoration::None);
  style_sheet.AddRule(StyleSelector::defaults(), default_style);

  Style marker_style;
  marker_style.set_color(Color(0x00, 0x99, 0x00));
  style_sheet.AddRule(StyleSelector::end_of_file_marker(), marker_style);
  style_sheet.AddRule(StyleSelector::end_of_line_marker(), marker_style);
  style_sheet.AddRule(StyleSelector::line_wrap_marker(), marker_style);
  style_sheet.AddRule(StyleSelector::tab_marker(), marker_style);

  // EvEdit 1.0's highlight color
  //selection_->SetColor(Color(0, 0, 0));
  //selection_->SetBackground(Color(0xCC, 0xCC, 0xFF));

  // We should not use GetSysColor. If we want to use here
  // default background must be obtained from GetSysColor.
  //selection_->SetColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
  //selection_->SetBackground(::GetSysColor(COLOR_HIGHLIGHT));

  // We use Vista's highlight color.
  Style active_selection;
  active_selection.set_color(Color(255, 255, 255));
  active_selection.set_color(Color(51, 153, 255));
  style_sheet.AddRule(StyleSelector::active_selection(), active_selection);

  Style inactive_selection;
  inactive_selection.set_color(Color(67, 78, 84));
  inactive_selection.set_color(Color(191, 205, 219));
  style_sheet.AddRule(StyleSelector::inactive_selection(), inactive_selection);

  default_style_sheet = &style_sheet;
  return default_style_sheet;
}


const Style& StyleSheet::Resolve(const base::string16& selector) const {
  auto const cache = style_cache_.find(selector);
  if (cache != style_cache_.end())
    return *cache->second;

  auto new_style = std::make_unique<Style>();
  for (auto runner = this; runner; runner = runner->base_style_sheet_) {
    auto it = runner->selector_map_.find(selector);
    if (it != runner->selector_map_.end())
      new_style->Merge(*it->second);
  }
  if (selector != StyleSelector::defaults())
    new_style->Merge(Resolve(StyleSelector::defaults()));
  auto const new_style_ptr = new_style.get();
  style_cache_[selector] = std::move(new_style);
  return *new_style_ptr;
}

}  // namespace css
