// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_style_sheet_h)
#define INCLUDE_evita_text_style_sheet_h

#include <memory>
#include <unordered_map>

#include "base/strings/string16.h"
#include "evita/text/style.h"

namespace text {

class Style;

class StyleSheet {
  private: typedef std::unordered_map<base::string16, std::unique_ptr<Style>>
      StyleMap;

  private: StyleSheet* base_style_sheet_;
  private: StyleMap selector_map_;
  private: mutable StyleMap style_cache_;

  public: StyleSheet();
  private: StyleSheet(bool for_default_style_sheet);
  public: ~StyleSheet();

  public: void AddRule(const base::string16& selector, const Style& style);
  private: void ClearCache();
  private: static StyleSheet* DefaultStyleSheet();
  public: const Style& Resolve(const base::string16& selector) const;

  DISALLOW_COPY_AND_ASSIGN(StyleSheet);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_style_sheet_h)
