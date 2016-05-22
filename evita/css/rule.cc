// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/rule.h"

#include "evita/css/style.h"

namespace css {

//////////////////////////////////////////////////////////////////////
//
// Rule
//
Rule::Rule(const Selector& selector, std::unique_ptr<Style> style)
    : selector_(selector), style_(std::move(style)) {}

Rule::~Rule() = default;

bool Rule::operator<(const Rule& other) const {
  return selector_ < other.selector_;
}

}  // namespace css
