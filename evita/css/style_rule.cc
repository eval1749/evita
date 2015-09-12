// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/style_rule.h"

namespace css {

StyleRule::StyleRule(const common::AtomicString& selector,
                     const css::Style* style)
    : selector_(selector), style_(style) {}

StyleRule::~StyleRule() {}

}  // namespace css
