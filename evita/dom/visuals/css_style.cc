// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "evita/dom/visuals/css_style.h"

#include "common/maybe.h"
#include "evita/dom/converter.h"
#include "evita/dom/script_host.h"
#include "evita/dom/visuals/css_style_sheet_handle.h"
#include "evita/dom/visuals/css_value_parsers.h"
#include "evita/v8_glue/runner.h"
#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_editor.h"
#include "evita/visuals/css/values.h"

namespace dom {

template <typename T>
using Maybe = common::Maybe<T>;

using CssPropertyId = visuals::css::PropertyId;
using CssStyle = visuals::css::Style;
using CssStyleEditor = visuals::css::StyleEditor;

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

//////////////////////////////////////////////////////////////////////
//
// CSSStyle
//

// static
std::unique_ptr<CssStyle> CSSStyle::ConvertFromV8(
    v8::Local<v8::Context> context,
    v8::Local<v8::Map> raw_style) {
  auto style = std::make_unique<CssStyle>();
#define V(Name, name, type, ...)                                          \
  {                                                                       \
    Maybe<base::string16> maybe_##name##_text =                           \
        GetRawProperty(context, raw_style, CssPropertyId::Name);          \
    if (maybe_##name##_text.IsJust()) {                                   \
      Maybe<visuals::css::type> maybe_##name =                            \
          Parse##type(maybe_##name##_text.FromJust());                    \
      if (maybe_##name.IsJust())                                          \
        CssStyleEditor().Set##Name(style.get(), maybe_##name.FromJust()); \
    }                                                                     \
  }
  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V
  return std::move(style);
}

// static
v8::Local<v8::Map> CSSStyle::ConvertToV8(v8::Local<v8::Context> context,
                                         const CssStyle& style) {
  const auto& map = v8::Map::New(context->GetIsolate());
  const auto contains = style.contains();
  for (auto property_id = 0u; property_id < contains.size(); ++property_id) {
    if (!contains.test(property_id))
      continue;
    switch (static_cast<CssPropertyId>(property_id)) {
#define V(Name, name, type, text)                     \
  case CssPropertyId::Name:                           \
    SetRawProperty(context, map, CssPropertyId::Name, \
                   Unparse##type(style.##name()));    \
    break;

      FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V
      default:
        NOTREACHED() << "Unhandled property: " << property_id;
        break;
    }
  }
  return map;
}

}  // namespace dom
