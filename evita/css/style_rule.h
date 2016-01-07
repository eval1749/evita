// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_STYLE_RULE_H_
#define EVITA_CSS_STYLE_RULE_H_

#include "base/macros.h"
#include "common/strings/atomic_string.h"

namespace css {

class Style;

class StyleRule final {
 public:
  StyleRule(const common::AtomicString& selector, const Style* style);
  ~StyleRule();

  const common::AtomicString& selector() const { return selector_; }
  const Style* style() const { return style_; }

 private:
  const common::AtomicString selector_;
  const Style* const style_;

  DISALLOW_COPY_AND_ASSIGN(StyleRule);
};

}  // namespace css

#endif  // EVITA_CSS_STYLE_RULE_H_
