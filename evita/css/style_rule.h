// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_css_style_rule_h)
#define INCLUDE_evita_css_style_rule_h

#include "base/strings/string16.h"

namespace css {

class Style;

class StyleRule {
  private: const base::string16 selector_;
  private: const Style* const style_;

  public: StyleRule(const base::string16& selector, const Style* style);
  public: virtual ~StyleRule();

  public: const base::string16& selector() const { return selector_; }
  public: const Style* style() const { return style_; }

  DISALLOW_COPY_AND_ASSIGN(StyleRule);
};

}  // namespace css

#endif //!defined(INCLUDE_evita_css_style_rule_h)
