// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/style_resolver.h"

#include "base/logging.h"
#include "evita/css/style.h"
#include "evita/css/style_rule.h"
#include "evita/css/style_selector.h"
#include "evita/css/style_sheet.h"

namespace css {

namespace {
StyleSheet* GetDefaultStyleSheet() {
  DEFINE_STATIC_LOCAL(StyleSheet, default_style_sheet, ());
  if (default_style_sheet.Find(StyleSelector::defaults()))
    return &default_style_sheet;

  Style default_style(Color(0, 0, 0), Color(255, 255, 255));
  default_style.set_font_family(L"Consolas, MS Gothic");
  default_style.set_font_size(10);
  default_style.set_font_style(css::FontStyle::Normal);
  default_style.set_font_weight(css::FontWeight::Normal);
  // TODO(yosi) We should move Style::syntax() to another place.
  default_style.set_syntax(0);
  default_style.set_text_decoration(css::TextDecoration::None);
  default_style_sheet.AddRule(StyleSelector::defaults(), default_style);

  Style marker_style;
  marker_style.set_color(Color(0x00, 0x99, 0x00));
  default_style_sheet.AddRule(StyleSelector::end_of_file_marker(),
                              marker_style);
  default_style_sheet.AddRule(StyleSelector::end_of_line_marker(),
                              marker_style);
  default_style_sheet.AddRule(StyleSelector::line_wrap_marker(), marker_style);
  default_style_sheet.AddRule(StyleSelector::tab_marker(), marker_style);

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
  default_style_sheet.AddRule(StyleSelector::active_selection(),
                              active_selection);

  Style inactive_selection;
  inactive_selection.set_color(Color(67, 78, 84));
  inactive_selection.set_color(Color(191, 205, 219));
  default_style_sheet.AddRule(StyleSelector::inactive_selection(),
                              inactive_selection);

  return &default_style_sheet;
}
}  // namespace

StyleResolver::StyleResolver() {
  AddStyleSheet(GetDefaultStyleSheet());
}

StyleResolver::~StyleResolver() {
  for (auto style_sheet : style_sheets_) {
    style_sheet->RemoveObserver(this);
  }
}

void StyleResolver::AddStyleSheet(const StyleSheet* style_sheet) {
  style_sheets_.push_back(style_sheet);
  style_sheet->AddObserver(this);
  ClearCache();
}

void StyleResolver::ClearCache() {
  style_cache_.clear();
}

void StyleResolver::RemoveStyleSheet(const StyleSheet* style_sheet) {
  auto it = std::find(style_sheets_.begin(), style_sheets_.end(), style_sheet);
  if (it != style_sheets_.end()) {
    style_sheets_.erase(it);
    style_sheet->RemoveObserver(this);
  }
  ClearCache();
}

const Style& StyleResolver::Resolve(const base::string16& selector) const {
  auto const cache = style_cache_.find(selector);
  if (cache != style_cache_.end())
    return *cache->second;

  auto new_style = std::make_unique<Style>();
  for (auto style_sheet : style_sheets_) {
    if (auto const style = style_sheet->Find(selector))
      new_style->OverrideBy(*style);
  }
  if (selector != StyleSelector::defaults())
    new_style->Merge(Resolve(StyleSelector::defaults()));
  auto const new_style_ptr = new_style.get();
  style_cache_[selector] = std::move(new_style);
  return *new_style_ptr;
}

// css::StyleSheetObserver
void StyleResolver::DidAddRule(const StyleRule* rule) {
  if (rule->selector() == StyleSelector::defaults()) {
    ClearCache();
  } else {
    auto it = style_cache_.find(rule->selector());
    if (it != style_cache_.end())
      style_cache_.erase(it);
  }
}

void StyleResolver::DidRemoveRule(const StyleRule* rule) {
  if (rule->selector() == StyleSelector::defaults()) {
    ClearCache();
  } else {
    auto it = style_cache_.find(rule->selector());
    if (it != style_cache_.end())
      style_cache_.erase(it);
  }
}

}  // namespace css
