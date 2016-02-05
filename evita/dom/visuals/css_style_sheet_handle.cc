// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/visuals/css_style_sheet_handle.h"

#include "base/logging.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/converter.h"
#include "evita/dom/script_host.h"
#include "evita/dom/visuals/css_style.h"
#include "evita/ginx/runner.h"
#include "evita/visuals/css/rule.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_sheet.h"

namespace dom {

using CssStyle = visuals::css::Style;

//////////////////////////////////////////////////////////////////////
//
// CSSStyleSheetHandle
//
CSSStyleSheetHandle::CSSStyleSheetHandle() : object_(new CssStyleSheet()) {}

CSSStyleSheetHandle::~CSSStyleSheetHandle() {}

// Bindings implementations
void CSSStyleSheetHandle::AppendStyleRule(CSSStyleSheetHandle* handle,
                                          const base::string16& selector,
                                          v8::Local<v8::Map> raw_style) {
  const auto& runner = ScriptHost::instance()->runner();
  const auto& context = runner->context();
  auto style = CSSStyle::ConvertFromV8(context, raw_style);
  handle->object_->AppendRule(selector, std::move(style));
}

int CSSStyleSheetHandle::CountRules(CSSStyleSheetHandle* handle) {
  return static_cast<int>(handle->object_->rules().size());
}

void CSSStyleSheetHandle::DeleteRule(CSSStyleSheetHandle* handle, int index) {
  handle->object_->RemoveRule(index);
}

void CSSStyleSheetHandle::InsertStyleRule(CSSStyleSheetHandle* handle,
                                          const base::string16& selector,
                                          v8::Local<v8::Map> raw_style,
                                          int index) {
  const auto& runner = ScriptHost::instance()->runner();
  const auto& context = runner->context();
  auto style = CSSStyle::ConvertFromV8(context, raw_style);
  handle->object_->InsertRule(selector, std::move(style), index);
}

v8::Local<v8::Map> CSSStyleSheetHandle::RuleAt(
    CSSStyleSheetHandle* handle,
    int index,
    ExceptionState* exception_state) {
  if (static_cast<size_t>(index) >= handle->object_->rules().size()) {
    exception_state->ThrowError("Unbound index");
    return v8::Local<v8::Map>();
  }
  const auto& runner = ScriptHost::instance()->runner();
  ginx::Runner::EscapableHandleScope runner_scope(runner);
  const auto& isolate = runner->isolate();
  const auto& context = runner->context();
  const auto& rule = handle->object_->rules()[index];
  const auto& map = CSSStyle::ConvertToV8(context, rule->style());
  return runner_scope.Escape(
      map->Set(context, gin::ConvertToV8(isolate, -1),
               gin::ConvertToV8(isolate, rule->selector()))
          .ToLocalChecked());
}

}  // namespace dom
