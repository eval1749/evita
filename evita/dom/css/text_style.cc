// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "base/strings/stringprintf.h"
#include "common/memory/singleton.h"
#include "evita/css/style.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/script_host.h"
#include "evita/dom/text/text_document.h"
#include "evita/dom/text/text_range.h"
#include "evita/dom/v8_strings.h"
#include "evita/text/buffer.h"
#include "evita/text/range.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/runner.h"

namespace dom {
namespace internal {
class EnumValue {
 public:
  template <typename T>
  v8::Maybe<T> FromV8(v8::Isolate* isolate,
                      v8::Handle<v8::Value> js_value) const {
    auto index = 0u;
    for (const auto& value : values_) {
      if (EqualNames(js_value, value->Get(isolate)))
        return v8::Just(static_cast<T>(index));
      ++index;
    }
    return v8::Nothing<T>();
  }

  template <typename T>
  v8::Handle<v8::Value> ToV8(v8::Isolate* isolate, T index) {
    if (static_cast<size_t>(index) >= values_.size())
      return v8::String::Empty(isolate);
    return values_[static_cast<size_t>(index)]->Get(isolate);
  }

 protected:
  EnumValue() = default;
  ~EnumValue() = default;

  void AddValue(v8::Eternal<v8::String>* value) { values_.push_back(value); }

 private:
  std::vector<v8::Eternal<v8::String>*> values_;

  DISALLOW_COPY_AND_ASSIGN(EnumValue);
};

class FontStyleValue final : public common::Singleton<FontStyleValue>,
                             public EnumValue {
  DECLARE_SINGLETON_CLASS(FontStyleValue);

 public:
  ~FontStyleValue() final = default;

 private:
  FontStyleValue() {
    AddValue(&v8Strings::normal);
    AddValue(&v8Strings::italic);
  }

  DISALLOW_COPY_AND_ASSIGN(FontStyleValue);
};

class FontWeightValue final : public common::Singleton<FontWeightValue>,
                              public EnumValue {
  DECLARE_SINGLETON_CLASS(FontWeightValue);

 public:
  ~FontWeightValue() final = default;

 private:
  FontWeightValue() {
    AddValue(&v8Strings::normal);
    AddValue(&v8Strings::bold);
  }

  DISALLOW_COPY_AND_ASSIGN(FontWeightValue);
};

class TextDecorationValue : public common::Singleton<TextDecorationValue>,
                            public EnumValue {
  DECLARE_SINGLETON_CLASS(TextDecorationValue);

 public:
  ~TextDecorationValue() final = default;

 private:
  TextDecorationValue() {
    AddValue(&v8Strings::none);
    AddValue(&v8Strings::greenwave);
    AddValue(&v8Strings::redwave);
    AddValue(&v8Strings::underline);
    AddValue(&v8Strings::imeinput);
    AddValue(&v8Strings::imeactive);
    AddValue(&v8Strings::imeinactive1);
    AddValue(&v8Strings::imeinactive2);
  }

  DISALLOW_COPY_AND_ASSIGN(TextDecorationValue);
};

}  // namespace internal
}  // namespace dom

namespace gin {

template <>
struct Converter<css::Color> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Handle<v8::Value> js_value,
                     css::Color* out_color) {
    int int_value;
    if (!ConvertFromV8(isolate, js_value, &int_value))
      return false;
    // TODO(eval1749): How do we represent css::Color in JS?
    *out_color = css::Color((int_value >> 16) & 0xFF, (int_value >> 8) & 0xFF,
                            int_value & 0xFF);
    return true;
  }
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate, css::Color color) {
    // TODO(eval1749): How do we represent css::Color in JS?
    const auto int_value =
        (color.red() << 16) | (color.green() << 8) | color.blue();
    return ConvertToV8(isolate, int_value);
  }
};

template <typename T, typename U>
struct EnumConverter {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Handle<v8::Value> js_value,
                     T* out_enum_value) {
    const auto maybe_enum_value = U::instance()->FromV8<T>(isolate, js_value);
    if (maybe_enum_value.IsNothing())
      return false;
    *out_enum_value = maybe_enum_value.FromJust();
    return true;
  }
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate, T enum_value) {
    return U::instance()->ToV8(isolate, enum_value);
  }
};

template <>
struct Converter<css::FontStyle>
    : EnumConverter<css::FontStyle, dom::internal::FontStyleValue> {};

template <>
struct Converter<css::FontWeight>
    : EnumConverter<css::FontWeight, dom::internal::FontWeightValue> {};

template <>
struct Converter<css::TextDecoration>
    : EnumConverter<css::TextDecoration, dom::internal::TextDecorationValue> {};

}  // namespace gin

namespace dom {

base::string16 V8ToString(v8::Handle<v8::Value> value);

namespace {
template <typename T>
v8::Maybe<T> ConvertFromV8(v8::Isolate* isolate,
                           v8::Handle<v8::Value> js_value) {
  if (js_value.IsEmpty())
    return v8::Nothing<T>();
  T cxx_value;
  return gin::ConvertFromV8(isolate, js_value, &cxx_value)
             ? v8::Just<T>(cxx_value)
             : v8::Nothing<T>();
}

bool EqualNames(v8::Handle<v8::Value> name1, v8::Handle<v8::String> name2) {
  v8::String::Value name_value1(name1);
  v8::String::Value name_value2(name2);
  if (name_value1.length() != name_value2.length())
    return false;
  return !::memcmp(*name_value1, *name_value2,
                   name_value1.length() * sizeof(uint16_t));
}

void InvalidStyleAttributeValue(ExceptionState* exception_state,
                                v8::Handle<v8::String> attr_name,
                                v8::Handle<v8::Value> attr_value) {
  exception_state->ThrowError(base::StringPrintf(
      "Style propery '%ls' doesn't take '%ls'.", V8ToString(attr_name).c_str(),
      V8ToString(attr_value).c_str()));
}
}  // namespace

v8::Handle<v8::Object> TextDocument::style_at(
    text::Offset position,
    ExceptionState* exception_state) const {
  if (!IsValidPosition(position, exception_state))
    return v8::Handle<v8::Object>();
  const auto& style_values = buffer_->GetStyleAt(position);

  const auto runner = ScriptHost::instance()->runner();
  const auto isolate = runner->isolate();
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);

  const auto style_ctor =
      runner->global()->Get(v8Strings::TextStyle.Get(isolate))->ToObject();
  const auto js_style = style_ctor->CallAsConstructor(0, nullptr)->ToObject();

  if (style_values.has_bgcolor()) {
    js_style->Set(v8Strings::backgroundColor.Get(isolate),
                  gin::ConvertToV8(isolate, style_values.bgcolor()));
  }

  if (style_values.has_color()) {
    js_style->Set(v8Strings::color.Get(isolate),
                  gin::ConvertToV8(isolate, style_values.color()));
  }

  if (style_values.has_text_decoration()) {
    js_style->Set(v8Strings::textDecoration.Get(isolate),
                  gin::ConvertToV8(isolate, style_values.text_decoration()));
  }

  if (style_values.has_font_size()) {
    js_style->Set(v8Strings::fontSize.Get(isolate),
                  v8::Number::New(isolate, style_values.font_size()));
  }

  if (style_values.has_font_style()) {
    js_style->Set(v8Strings::fontStyle.Get(isolate),
                  gin::ConvertToV8(isolate, style_values.font_style()));
  }

  if (style_values.has_font_weight()) {
    js_style->Set(v8Strings::fontWeight.Get(isolate),
                  gin::ConvertToV8(isolate, style_values.font_weight()));
  }

  return runner_scope.Escape(js_style);
}

void TextRange::SetStyle(v8::Handle<v8::Object> style_dict,
                         ExceptionState* exception_state) const {
  css::Style style_values;
  bool changed = false;

  const auto runner = ScriptHost::instance()->runner();
  const auto isolate = runner->isolate();
  v8_glue::Runner::Scope runner_scope(runner);

#define LOAD_DICT_VALUE(type, attr_name, member_name)                 \
  const auto attr_name = v8Strings::attr_name.Get(isolate);           \
  if (EqualNames(name, attr_name)) {                                  \
    const auto js_value = style_dict->Get(attr_name);                 \
    if (js_value->IsUndefined())                                      \
      continue;                                                       \
    const auto attr_value = ConvertFromV8<type>(isolate, js_value);   \
    if (attr_value.IsJust()) {                                        \
      style_values.set_##member_name(attr_value.FromJust());          \
      changed = true;                                                 \
      continue;                                                       \
    }                                                                 \
    InvalidStyleAttributeValue(exception_state, attr_name, js_value); \
    return;                                                           \
  }

  const auto names = style_dict->GetPropertyNames();
  const auto names_length = names->Length();
  for (auto index = 0u; index < names_length; ++index) {
    const auto name = names->Get(index);
    if (name.IsEmpty())
      continue;
    LOAD_DICT_VALUE(css::Color, backgroundColor, bgcolor);
    LOAD_DICT_VALUE(css::Color, color, color);
    LOAD_DICT_VALUE(css::FontSize, fontSize, font_size);
    LOAD_DICT_VALUE(css::FontStyle, fontStyle, font_style);
    LOAD_DICT_VALUE(css::FontWeight, fontWeight, font_weight);
    LOAD_DICT_VALUE(css::TextDecoration, textDecoration, text_decoration)

    exception_state->ThrowError(base::StringPrintf(
        "Invalid style attribute name '%ls'", V8ToString(name).c_str()));
    return;
  }
  if (!changed)
    return;
  document_->buffer()->SetStyle(range_->start(), range_->end(), style_values);
}

}  // namespace dom
