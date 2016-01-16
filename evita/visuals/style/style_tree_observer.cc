// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/style_tree_observer.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// StyleTreeObserver
//
StyleTreeObserver::StyleTreeObserver() {}
StyleTreeObserver::~StyleTreeObserver() {}

void StyleTreeObserver::DidChangeComputedStyle(const ElementNode& element,
                                               const css::Style& old_style) {}

}  // namespace visuals
