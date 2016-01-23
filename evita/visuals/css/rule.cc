// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/rule.h"

#include "evita/visuals/css/style.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Rule
//
Rule::Rule(base::StringPiece16 selector, std::unique_ptr<Style> style)
    : selector_(selector.as_string()), style_(std::move(style)) {}

Rule::~Rule() {}

}  // namespace css
}  // namespace visuals
