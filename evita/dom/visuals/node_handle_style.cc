// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "evita/dom/visuals/node_handle.h"

#include "common/maybe.h"
#include "evita/dom/converter.h"
#include "evita/dom/script_host.h"
#include "evita/dom/visuals/css_value_parsers.h"
#include "evita/v8_glue/runner.h"
#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_editor.h"
#include "evita/visuals/css/values.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_editor.h"

namespace dom {

using CssPropertyId = visuals::css::PropertyId;
using CssStyle = visuals::css::Style;
using CssStyleEditor = visuals::css::StyleEditor;
using Element = visuals::Element;

template <typename T>
using Maybe = common::Maybe<T>;

base::string16 V8ToString(v8::Local<v8::Value> value);

namespace {

Maybe<base::string16> GetRawProperty(v8::Local<v8::Context> context,
                                     v8::Local<v8::Map> map,
                                     CssPropertyId property_id) {
  v8::Local<v8::Value> key =
      gin::ConvertToV8(context->GetIsolate(), static_cast<int>(property_id));
  if (!map->Has(context, key).FromMaybe(false))
    return common::Nothing<base::string16>();
  v8::Local<v8::Value> value = map->Get(context, key).ToLocalChecked();
  if (value.IsEmpty())
    return common::Nothing<base::string16>();
  return common::Just<base::string16>(V8ToString(value));
}

void SetRawProperty(v8::Local<v8::Context> context,
                    v8::Local<v8::Map> map,
                    CssPropertyId property_id,
                    const base::string16& value) {
  const auto& isolate = context->GetIsolate();
  const auto& result = map->Set(context, gin::ConvertToV8(isolate, property_id),
                                gin::ConvertToV8(isolate, value))
                           .ToLocalChecked();
  if (!result.IsEmpty())
    return;
  ScriptHost::instance()->ThrowError("Failed to set Map.");
}

}  // namespace

v8::Handle<v8::Map> NodeHandle::GetInlineStyle(NodeHandle* handle) {
  const auto& runner = ScriptHost::instance()->runner();
  const auto& isolate = runner->isolate();
  const auto& context = runner->context();
  const auto& element = handle->value()->as<Element>();
  if (!element) {
    ScriptHost::instance()->ThrowError("Requires element object");
    return v8::Handle<v8::Map>();
  }
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);
  const auto map = v8::Map::New(isolate);
  const auto& style = element->inline_style();
  if (!style)
    return runner_scope.Escape(map);
#define V(Name, name, type, text)                     \
  if (style->has_##name())                            \
    SetRawProperty(context, map, CssPropertyId::Name, \
                   Unparse##Name(style->##name()));
  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V
  return runner_scope.Escape(map);
}

void NodeHandle::SetInlineStyle(NodeHandle* handle, v8::Handle<v8::Map> map) {
  const auto& runner = ScriptHost::instance()->runner();
  const auto& context = runner->context();
  const auto element = handle->value()->as<Element>();
  if (!element) {
    ScriptHost::instance()->ThrowError("Requires element object");
    return;
  }
  auto style = std::make_unique<CssStyle>();
#define V(Name, name, type, text)                                         \
  {                                                                       \
    Maybe<base::string16> maybe_##name##_text =                           \
        GetRawProperty(context, map, CssPropertyId::Name);                \
    if (maybe_##name##_text.IsJust()) {                                   \
      Maybe<visuals::css::Name> maybe_##name =                            \
          Parse##Name(maybe_##name##_text.FromJust());                    \
      if (maybe_##name.IsJust())                                          \
        CssStyleEditor().Set##Name(style.get(), maybe_##name.FromJust()); \
    }                                                                     \
  }
  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V
  visuals::NodeEditor().SetInlineStyle(element, *style);
}

}  // namespace dom
