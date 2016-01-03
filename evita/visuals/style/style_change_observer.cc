// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/style_change_observer.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// StyleChangeObserver
//
StyleChangeObserver::StyleChangeObserver() {}
StyleChangeObserver::~StyleChangeObserver() {}

void StyleChangeObserver::DidClearStyleCache() {}

void StyleChangeObserver::DidRemoveStyleCache(const Element& element,
                                              const css::Style& old_style) {}

}  // namespace visuals
