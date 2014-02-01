// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/strings/stringprintf.h"
#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/dom/script_controller.h"
#include "evita/ed_style.h"
#include "evita/v8_glue/converter.h"
#include "v8_strings.h"

namespace gin {

static const char* const decoration_values[] = {
  "none",
  "greenwave",
  "redwave",
  "underline",
  "imeinput",
  "imeactive",
  "imeinactive1",
  "imeinactive2",
};

static const char* font_style_values[] = {
  "normal",
  "italic",
};

static const char* font_weight_values[] = {
  "normal",
  "bold",
};

template<>
struct Converter<FontStyle> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    FontStyle font_style) {
    if (static_cast<size_t>(font_style) >= arraysize(font_style_values))
      return v8::String::Empty(isolate);
    return StringToSymbol(isolate, font_style_values[font_style]);
  }
};

template<>
struct Converter<FontWeight> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    FontWeight font_weight) {
    if (static_cast<size_t>(font_weight) >= arraysize(font_weight_values))
      return v8::String::Empty(isolate);
    return StringToSymbol(isolate, font_weight_values[font_weight]);
  }
};

template<>
struct Converter<TextDecoration> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    TextDecoration decoration) {
    if (static_cast<size_t>(decoration) >= arraysize(decoration_values))
      return v8::String::Empty(isolate);
    return StringToSymbol(isolate, decoration_values[decoration]);
  }
};

}  // namespace gin

namespace dom {

v8::Handle<v8::Object> Document::style_at(text::Posn position) const {
  if (!IsValidPosition(position))
    return v8::Handle<v8::Object>();
  auto const style_attrs = buffer_->GetStyleAt(position);

  auto const isolate = v8::Isolate::GetCurrent();
  v8::EscapableHandleScope handle_scope(isolate);
  auto const context = ScriptController::instance()->context();
  v8::Context::Scope context_scope(context);
  auto const style_ctor = context->Global()->Get(
      v8Strings::Style.Get(isolate))->ToObject();
  auto const style = style_ctor->CallAsConstructor(0, nullptr)->ToObject();

  if (style_attrs->m_rgfMask  & StyleValues::Mask_Background) {
    style->Set(v8Strings::backgroundColor.Get(isolate),
               v8::Integer::New(isolate,
                   static_cast<int>(style_attrs->GetBackground())));
  }

  if (style_attrs->m_rgfMask  & StyleValues::Mask_Color) {
    style->Set(v8Strings::color.Get(isolate),
               v8::Integer::New(isolate,
                    static_cast<int>(style_attrs->GetColor())));
  }

  if (style_attrs->m_rgfMask & StyleValues::Mask_Decoration) {
    style->Set(v8Strings::fontWeight.Get(isolate),
               gin::ConvertToV8(isolate, style_attrs->GetDecoration()));
  }

  if (style_attrs->m_rgfMask  & StyleValues::Mask_FontSize) {
    style->Set(v8Strings::fontSize.Get(isolate),
               v8::Integer::New(isolate, style_attrs->GetFontSize()));
  }

  if (style_attrs->m_rgfMask & StyleValues::Mask_FontStyle) {
    style->Set(v8Strings::fontStyle.Get(isolate),
               gin::ConvertToV8(isolate, style_attrs->GetFontStyle()));
  }

  if (style_attrs->m_rgfMask & StyleValues::Mask_FontWeight) {
    style->Set(v8Strings::fontWeight.Get(isolate),
               gin::ConvertToV8(isolate, style_attrs->GetFontWeight()));
  }

  if (style_attrs->m_rgfMask & StyleValues::Mask_Syntax) {
    style->Set(v8Strings::charSyntax.Get(isolate),
               v8::Integer::New(isolate, style_attrs->GetSyntax()));
  }
  return handle_scope.Escape(style);
}

}  // namespace
