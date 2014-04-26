// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_css_style_sheet_h)
#define INCLUDE_evita_css_style_sheet_h

#include <memory>
#include <unordered_map>

#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/observer_list.h"
#pragma warning(pop)
#include "base/strings/string16.h"
#include "evita/css/style_sheet_observer.h"

namespace common {
class AtomicString;
}

namespace css {

class Style;

class StyleSheet {
  private: typedef std::unordered_map<const base::string16*,
                                      std::unique_ptr<Style>>
      StyleMap;

  private: ObserverList<StyleSheetObserver> observers_;
  private: StyleMap selector_map_;

  public: StyleSheet();
  public: ~StyleSheet();

  public: void AddObserver(StyleSheetObserver* observer) const;
  public: void AddRule(const base::string16& selector, const Style& style);
  public: void AddRule(const common::AtomicString& selector,
                       const Style& style);
  public: const Style* Find(const base::string16& selector) const;
  public: const Style* Find(const common::AtomicString& selector) const;
  public: void RemoveObserver(StyleSheetObserver* observer) const;

  DISALLOW_COPY_AND_ASSIGN(StyleSheet);
};

}  // namespace css

#endif //!defined(INCLUDE_evita_css_style_sheet_h)
