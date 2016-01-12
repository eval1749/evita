// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/visuals/css_style_sheet_handle.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// CSSStyleSheetHandle
//
CSSStyleSheetHandle::CSSStyleSheetHandle() : object_(new css::StyleSHeet()) {}

CSSStyleSheetHandle::~CSSStyleSheetHandle() {}

// Bindings implementations
int CSSStyleSheetHandle::CountRules() {
  return static_cast<int>(object_->rules().size());
}

void CSSStyleSheetHandle::DeleteRule(int index) {
  object_->RemoveRule(index);
}

void CSSStyleSheetHandle::InsertStyleRule(const base::string16& selector,
                                          v8::Local<v8::Map> raw_style,
                                          int index) {
  object_->InsertRule(selector, std::move(CreateStyle(raw_style)), index);
}

v8::Local<v8::Map> RuleAt(int index);
}  // namespace dom
