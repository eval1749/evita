// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/visuals/node_handle.h"

#include "common/maybe.h"
#include "evita/dom/script_host.h"
#include "evita/dom/visuals/css_style.h"
#include "evita/v8_glue/runner.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_editor.h"

namespace dom {

using Element = visuals::Element;

v8::Local<v8::Map> NodeHandle::GetInlineStyle(NodeHandle* handle) {
  const auto& runner = ScriptHost::instance()->runner();
  const auto& context = runner->context();
  const auto& element = handle->value()->as<Element>();
  if (!element) {
    ScriptHost::instance()->ThrowError("Requires element object");
    return v8::Local<v8::Map>();
  }
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);
  const auto& style = element->inline_style();
  if (!style)
    return runner_scope.Escape(v8::Map::New(context->GetIsolate()));
  return runner_scope.Escape(CSSStyle::ConvertToV8(context, *style));
}

void NodeHandle::SetInlineStyle(NodeHandle* handle,
                                v8::Local<v8::Map> raw_style) {
  const auto& runner = ScriptHost::instance()->runner();
  const auto& context = runner->context();
  const auto element = handle->value()->as<Element>();
  if (!element) {
    ScriptHost::instance()->ThrowError("Requires element object");
    return;
  }
  auto style = CSSStyle::ConvertFromV8(context, raw_style);
  visuals::NodeEditor().SetInlineStyle(element, *style);
}

}  // namespace dom
