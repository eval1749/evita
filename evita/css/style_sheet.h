// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_STYLE_SHEET_H_
#define EVITA_CSS_STYLE_SHEET_H_

#include <memory>
#include <utility>
#include <vector>

#include "base/macros.h"
#include "base/observer_list.h"
#include "evita/gc/collectable.h"

namespace css {

class Rule;
class Selector;
class Style;
class StyleSheetObserver;

//////////////////////////////////////////////////////////////////////
//
// StyleSheet
//
class StyleSheet final : public gc::Collectable<StyleSheet> {
  DECLARE_GC_VISITABLE_OBJECT(StyleSheet)

 public:
  StyleSheet();
  ~StyleSheet();

  const std::vector<Rule*>& rules() const { return rules_; }

  void AppendRule(const Selector& selector, std::unique_ptr<css::Style> style);
  void AddObserver(StyleSheetObserver* observer) const;
  void InsertRule(const Selector& selector,
                  std::unique_ptr<css::Style> style,
                  size_t index);
  void RemoveObserver(StyleSheetObserver* observer) const;
  void RemoveRule(size_t index);

 private:
  mutable base::ObserverList<StyleSheetObserver> observers_;
  std::vector<Rule*> rules_;

  DISALLOW_COPY_AND_ASSIGN(StyleSheet);
};

}  // namespace css

#endif  // EVITA_CSS_STYLE_SHEET_H_
