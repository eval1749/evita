// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "evita/dom/css/css_style.h"

#include "evita/base/maybe.h"
#include "evita/css/properties.h"
#include "evita/css/style.h"
#include "evita/css/style_editor.h"
#include "evita/css/values.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/converter.h"
#include "evita/dom/css/css_style_sheet_handle.h"
#include "evita/dom/css/css_value_parsers.h"

namespace dom {

template <typename T>
using Maybe = base::Maybe<T>;

namespace {

Maybe<base::string16> GetRawProperty(v8::Local<v8::Context> context,
                                     v8::Local<v8::Map> map,
                                     css::PropertyId property_id) {
  v8::Local<v8::Value> key =
      gin::ConvertToV8(context->GetIsolate(), static_cast<int>(property_id));
  if (!map->Has(context, key).FromMaybe(false))
    return base::Nothing<base::string16>();
  v8::Local<v8::Value> value = map->Get(context, key).ToLocalChecked();
  if (value.IsEmpty())
    return base::Nothing<base::string16>();
  const auto& string = v8::String::Value(value);
  return base::Just<base::string16>(base::string16(
      reinterpret_cast<const wchar_t*>(*string), string.length()));
}

void SetRawProperty(v8::Local<v8::Context> context,
                    v8::Local<v8::Map> map,
                    const css::Property& property,
                    ExceptionState* exception_state) {
  const auto& isolate = context->GetIsolate();
  const auto& result =
      map->Set(context, gin::ConvertToV8(isolate, property.id()),
               gin::ConvertToV8(isolate, property.value().ToString16()))
          .ToLocalChecked();
  if (!result.IsEmpty())
    return;
  exception_state->ThrowError("Failed to set Map.");
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CSSStyle
//

// static
std::unique_ptr<css::Style> CSSStyle::ConvertFromV8(
    v8::Local<v8::Context> context,
    v8::Local<v8::Map> raw_style) {
  auto style = std::make_unique<css::Style>();
#define V(Name, name, type, ...)                                            \
  {                                                                         \
    Maybe<base::string16> maybe_##name##_text =                             \
        GetRawProperty(context, raw_style, css::PropertyId::Name);          \
    if (maybe_##name##_text.IsJust()) {                                     \
      Maybe<css::type> maybe_##name =                                       \
          Parse##type(maybe_##name##_text.FromJust());                      \
      if (maybe_##name.IsJust())                                            \
        css::StyleEditor().Set##Name(style.get(), maybe_##name.FromJust()); \
    }                                                                       \
  }
  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V
  return std::move(style);
}

// static
v8::Local<v8::Map> CSSStyle::ConvertToV8(v8::Local<v8::Context> context,
                                         const css::Style& style) {
  const auto& map = v8::Map::New(context->GetIsolate());
  for (const auto& property : style.properties()) {
    ExceptionState exception_state(ExceptionState::Situation::PropertySet,
                                   context, "CSSStyle", property.ascii_name());
    SetRawProperty(context, map, property, &exception_state);
  }
  return map;
}

}  // namespace dom
