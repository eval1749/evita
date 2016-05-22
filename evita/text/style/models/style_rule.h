// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_STYLE_MODELS_STYLE_RULE_H_
#define EVITA_TEXT_STYLE_MODELS_STYLE_RULE_H_

#include "base/macros.h"
#include "evita/base/strings/atomic_string.h"

namespace xcss {

class Style;

class StyleRule final {
 public:
  StyleRule(base::AtomicString selector, const Style* style);
  ~StyleRule();

  base::AtomicString selector() const { return selector_; }
  const Style* style() const { return style_; }

 private:
  const base::AtomicString selector_;
  const Style* const style_;

  DISALLOW_COPY_AND_ASSIGN(StyleRule);
};

}  // namespace xcss

#endif  // EVITA_TEXT_STYLE_MODELS_STYLE_RULE_H_
