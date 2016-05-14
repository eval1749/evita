// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_STYLE_MODELS_STYLE_SHEET_H_
#define EVITA_TEXT_STYLE_MODELS_STYLE_SHEET_H_

#include <memory>
#include <unordered_map>

#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "evita/base/strings/atomic_string.h"
#include "evita/text/style/models/style_sheet_observer.h"

namespace base {
class AtomicString;
}

namespace css {

class Style;

class StyleSheet final {
 public:
  StyleSheet();
  ~StyleSheet();

  void AddObserver(StyleSheetObserver* observer) const;
  void AddRule(const base::string16& selector, const Style& style);
  void AddRule(base::AtomicString selector, const Style& style);
  const Style* Find(const base::string16& selector) const;
  const Style* Find(base::AtomicString selector) const;
  void RemoveObserver(StyleSheetObserver* observer) const;

 private:
  using StyleMap =
      std::unordered_map<base::AtomicString, std::unique_ptr<Style>>;

  base::ObserverList<StyleSheetObserver> observers_;
  StyleMap selector_map_;

  DISALLOW_COPY_AND_ASSIGN(StyleSheet);
};

}  // namespace css

#endif  // EVITA_TEXT_STYLE_MODELS_STYLE_SHEET_H_
