// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/style_sheet_observer.h"

namespace css {

//////////////////////////////////////////////////////////////////////
//
// StyleSheetObserver
//
StyleSheetObserver::StyleSheetObserver() {}
StyleSheetObserver::~StyleSheetObserver() {}
void StyleSheetObserver::DidInsertRule(const Rule& new_rule, size_t index) {}
void StyleSheetObserver::DidRemoveRule(const Rule& old_rule, size_t index) {}

}  // namespace css
