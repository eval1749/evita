// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_STYLE_SHEET_H_
#define EVITA_VISUALS_CSS_STYLE_SHEET_H_

#include <list>
#include <memory>
#include <utility>

#include "base/macros.h"
#include "base/observer_list.h"
#include "base/strings/string_piece.h"
#include "evita/gc/collectable.h"

namespace visuals {
namespace css {

class Rule;
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

  const std::list<Rule*>& rules() const { return rules_; }

  void AddRule(const base::StringPiece16& selector,
               std::unique_ptr<css::Style> style);
  void AddObserver(StyleSheetObserver* observer) const;
  void RemoveObserver(StyleSheetObserver* observer) const;
  void RemoveRule(size_t index);

 private:
  mutable base::ObserverList<StyleSheetObserver> observers_;
  std::list<Rule*> rules_;

  DISALLOW_COPY_AND_ASSIGN(StyleSheet);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_STYLE_SHEET_H_
