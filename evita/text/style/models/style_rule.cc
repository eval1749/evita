// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/style/models/style_rule.h"

namespace xcss {

StyleRule::StyleRule(base::AtomicString selector, const xcss::Style* style)
    : selector_(selector), style_(style) {}

StyleRule::~StyleRule() {}

}  // namespace xcss
