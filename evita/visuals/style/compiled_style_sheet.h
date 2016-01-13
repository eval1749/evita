// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_COMPILED_STYLE_SHEET_H_
#define EVITA_VISUALS_STYLE_COMPILED_STYLE_SHEET_H_

#include <memory>
#include <unordered_map>

#include "base/macros.h"
#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "evita/visuals/css/style_sheet_observer.h"

namespace visuals {

class Element;

namespace css {
class Style;
class StyleSheet;
}

//////////////////////////////////////////////////////////////////////
//
// CompiledStyleSheet
//
class CompiledStyleSheet final : public css::StyleSheetObserver {
 public:
  explicit CompiledStyleSheet(const css::StyleSheet& style_sheet);
  ~CompiledStyleSheet();

  void AddObserver(css::StyleSheetObserver* observer) const;
  std::unique_ptr<css::Style> Match(const Element& element) const;
  void RemoveObserver(css::StyleSheetObserver* observer) const;

 private:
  void CompileRule(const css::Rule& rule);

  // css::StyleSheetObserver
  void DidInsertRule(const css::Rule& new_rule, size_t index);
  void DidRemoveRule(const css::Rule& old_rule, size_t index);

  mutable base::ObserverList<css::StyleSheetObserver> observers_;
  const css::StyleSheet& style_sheet_;

  std::unordered_map<base::string16, const css::Style*> class_name_map_;
  std::unordered_map<base::string16, const css::Style*> id_map_;
  std::unordered_map<base::string16, const css::Style*> tag_name_map_;

  DISALLOW_COPY_AND_ASSIGN(CompiledStyleSheet);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_COMPILED_STYLE_SHEET_H_
