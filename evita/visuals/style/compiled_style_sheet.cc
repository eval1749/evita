// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/compiled_style_sheet.h"

#include "base/strings/string16.h"
#include "evita/base/strings/atomic_string.h"
#include "evita/visuals/css/rule.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_editor.h"
#include "evita/visuals/css/style_sheet.h"
#include "evita/visuals/dom/element.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// StyleSheet
//
CompiledStyleSheet::CompiledStyleSheet(const css::StyleSheet& style_sheet)
    : style_sheet_(style_sheet) {
  for (const auto& rule : style_sheet_.rules())
    CompileRule(*rule);
  style_sheet_.AddObserver(this);
}

CompiledStyleSheet::~CompiledStyleSheet() {
  style_sheet_.RemoveObserver(this);
}

void CompiledStyleSheet::AddObserver(css::StyleSheetObserver* observer) const {
  observers_.AddObserver(observer);
}

void CompiledStyleSheet::CompileRule(const css::Rule& rule) {
  if (rule.selector()[0] == '#') {
    const auto& id = AtomicString(rule.selector().substr(1));
    const auto& insert_id = id_map_.emplace(id, &rule.style());
    DCHECK(insert_id.second);
    return;
  }

  if (rule.selector()[0] == '.') {
    const auto& class_name = AtomicString(rule.selector().substr(1));
    const auto& insert_class =
        class_name_map_.emplace(class_name, &rule.style());
    DCHECK(insert_class.second);
    return;
  }

  const auto& tag_name = AtomicString(rule.selector());
  const auto& insert_tag = tag_name_map_.emplace(tag_name, &rule.style());
  DCHECK(insert_tag.second);
}

std::unique_ptr<css::Style> CompiledStyleSheet::Match(
    const ElementNode& element) const {
  auto matched = false;
  css::Style style;
  if (!element.id().empty()) {
    const auto& id_it = id_map_.find(element.id());
    if (id_it != id_map_.end()) {
      matched = true;
      style = *id_it->second;
    }
  }

  for (const auto& class_name : element.class_list()) {
    const auto& class_name_it = class_name_map_.find(class_name);
    if (class_name_it != class_name_map_.end()) {
      css::StyleEditor().Merge(&style, *class_name_it->second);
      matched = true;
    }
  }

  const auto& tag_name_it = tag_name_map_.find(element.tag_name());
  if (tag_name_it != tag_name_map_.end()) {
    css::StyleEditor().Merge(&style, *tag_name_it->second);
    matched = true;
  }
  if (!matched)
    return std::unique_ptr<css::Style>();
  return std::make_unique<css::Style>(style);
}

void CompiledStyleSheet::RemoveObserver(
    css::StyleSheetObserver* observer) const {
  observers_.RemoveObserver(observer);
}

// css::StyleSheetObserver
void CompiledStyleSheet::DidInsertRule(const css::Rule& new_rule,
                                       size_t index) {
  FOR_EACH_OBSERVER(css::StyleSheetObserver, observers_,
                    DidInsertRule(new_rule, index));
}

void CompiledStyleSheet::DidRemoveRule(const css::Rule& old_rule,
                                       size_t index) {
  FOR_EACH_OBSERVER(css::StyleSheetObserver, observers_,
                    DidRemoveRule(old_rule, index));
}

}  // namespace visuals
