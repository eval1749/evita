// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "base/strings/stringprintf.h"
#include "common/memory/singleton.h"
#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/dom/range.h"
#include "evita/dom/script_controller.h"
#include "evita/ed_style.h"
#include "evita/text/range.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/runner.h"
#include "v8_strings.h"

namespace dom {
namespace internal {
class EnumValue {
  private: std::vector<v8::Eternal<v8::String>*> values_;

  protected: EnumValue() = default;
  protected: ~EnumValue() = default;

  protected: void AddValue(v8::Eternal<v8::String>* value) {
    values_.push_back(value);
  }

  public: template<typename T> v8::Maybe<T> FromV8(
      v8::Isolate* isolate, v8::Handle<v8::Value> js_value) const {
    auto index = 0u;
    for (auto value : values_) {
      if (EqualNames(js_value, value->Get(isolate)))
        return v8::Maybe<T>(static_cast<T>(index));
      ++index;
    }
    return v8::Maybe<T>();
  }

  public: template<typename T> v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                                          T index) {
    if (static_cast<size_t>(index) >= values_.size())
      return v8::String::Empty(isolate);
    return values_[index]->Get(isolate);
  }
};

class FontStyleValue : public common::Singleton<FontStyleValue>,
                       public EnumValue {
  DECLARE_SINGLETON_CLASS(FontStyleValue);

  private: FontStyleValue() {
    AddValue(&v8Strings::normal);
    AddValue(&v8Strings::italic);
  }

  public: ~FontStyleValue() = default;
};

class FontWeightValue : public common::Singleton<FontWeightValue>,
                       public EnumValue {
  DECLARE_SINGLETON_CLASS(FontWeightValue);

  private: FontWeightValue() {
    AddValue(&v8Strings::normal);
    AddValue(&v8Strings::bold);
  }

  public: ~FontWeightValue() = default;
};

class TextDecorationValue : public common::Singleton<TextDecorationValue>,
                       public EnumValue {
  DECLARE_SINGLETON_CLASS(TextDecorationValue);

  private: TextDecorationValue() {
    AddValue(&v8Strings::none);
    AddValue(&v8Strings::greenwave);
    AddValue(&v8Strings::redwave);
    AddValue(&v8Strings::underline);
    AddValue(&v8Strings::imeinput);
    AddValue(&v8Strings::imeactive);
    AddValue(&v8Strings::imeinactive1);
    AddValue(&v8Strings::imeinactive2);
  }

  public: ~TextDecorationValue() = default;
};

}  // namespace internal
}  // namespace dom

namespace gin {

template<>
struct Converter<Color> {
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> js_value,
                     Color* out_color) {
    int int_value;
    if (!ConvertFromV8(isolate, js_value, &int_value))
      return false;
    *out_color = Color(static_cast<COLORREF>(int_value));
    return true;
  }
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate, Color color) {
    return ConvertToV8(isolate, static_cast<int>(static_cast<COLORREF>(color)));
  }
};

template<typename T, typename U>
struct EnumConverter {
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> js_value,
                     T* out_enum_value) {
    auto const maybe_enum_value = U::instance()->FromV8<T>(isolate, js_value);
    if (!maybe_enum_value.has_value)
      return false;
    *out_enum_value = maybe_enum_value.value;
    return true;
  }
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate, T enum_value) {
    return U::instance()->ToV8(isolate, enum_value);
  }
};

template<>
struct Converter<FontStyle> :
    EnumConverter<FontStyle, dom::internal::FontStyleValue> {
};

template<>
struct Converter<FontWeight> :
    EnumConverter<FontWeight, dom::internal::FontWeightValue> {
};


template<>
struct Converter<TextDecoration> :
    EnumConverter<TextDecoration, dom::internal::TextDecorationValue> {
};

}  // namespace gin

namespace dom {

base::string16 V8ToString(v8::Handle<v8::Value> value);

namespace {
template<typename T>
v8::Maybe<T> ConvertFromV8(v8::Isolate* isolate,
                           v8::Handle<v8::Value> js_value) {
  if (js_value.IsEmpty())
    return v8::Maybe<T>();
  T cxx_value;
  return gin::ConvertFromV8(isolate, js_value, &cxx_value) ?
      v8::Maybe<T>(true, cxx_value) : v8::Maybe<T>();
}

bool EqualNames(v8::Handle<v8::Value> name1, v8::Handle<v8::String> name2) {
  v8::String::Value name_value1(name1);
  v8::String::Value name_value2(name2);
  if (name_value1.length() != name_value2.length())
    return false;
  return !::memcmp(*name_value1, *name_value2,
                   name_value1.length() * sizeof(uint16_t));
}

void InvalidStyleAttributeValue(v8::Isolate* isolate,
                                v8::Handle<v8::String> attr_name) {
  ScriptController::instance()->ThrowException(v8::Exception::Error(
      gin::StringToV8(isolate, base::StringPrintf(
          L"Invalid value for style attribute '%ls'",
          V8ToString(attr_name).c_str()))));
}
}

v8::Handle<v8::Object> Document::style_at(text::Posn position) const {
  if (!IsValidPosition(position))
    return v8::Handle<v8::Object>();
  auto const style_attrs = buffer_->GetStyleAt(position);

  auto const runner = ScriptController::instance()->runner();
  auto const isolate = runner->isolate();
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);

  auto const style_ctor = runner->global()->Get(
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
    style->Set(v8Strings::textDecoration.Get(isolate),
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
  return runner_scope.Escape(style);
}

void Range::SetStyle(v8::Handle<v8::Object> style_dict) const {
  StyleValues style_attrs;
  style_attrs.m_rgfMask = 0;

  auto const runner = ScriptController::instance()->runner();
  auto const isolate = runner->isolate();
  v8_glue::Runner::Scope runner_scope(runner);

  #define LOAD_DICT_VALUE(type, attr_name, mask_name, member_name) \
    auto const attr_name = v8Strings::attr_name.Get(isolate); \
    if (EqualNames(name, attr_name)) { \
      auto const js_value = style_dict->Get(attr_name); \
      if (js_value->IsUndefined()) \
        continue; \
      auto const attr_value = ConvertFromV8<type>(isolate, js_value); \
      if (attr_value.has_value) { \
        style_attrs.m_rgfMask |= StyleValues::Mask_ ## mask_name; \
        style_attrs.member_name = attr_value.value; \
        continue; \
      } \
      InvalidStyleAttributeValue(isolate, attr_name); \
      return; \
   } \

  auto const names = style_dict->GetPropertyNames();
  auto const names_length = names->Length();
  for (auto index = 0u; index < names_length; ++index) {
    auto const name = names->Get(index);
    if (name.IsEmpty())
      continue;
    LOAD_DICT_VALUE(Color, backgroundColor, Background, m_crBackground);
    LOAD_DICT_VALUE(int, charSyntax, Syntax, m_nSyntax);
    LOAD_DICT_VALUE(Color, color, Color, m_crColor);
    LOAD_DICT_VALUE(int, fontSize, FontSize, m_nFontSize);
    LOAD_DICT_VALUE(FontStyle, fontStyle, FontStyle, m_eFontStyle);
    LOAD_DICT_VALUE(FontWeight, fontWeight, FontWeight, m_eFontWeight);
    LOAD_DICT_VALUE(TextDecoration, textDecoration, Decoration,
                    m_eDecoration);

    ScriptController::instance()->ThrowException(v8::Exception::Error(
        gin::StringToV8(isolate, base::StringPrintf(
            L"Invalid style attribute name '%ls'",
            V8ToString(name).c_str()))));
    return;
  }
  if (!style_attrs.m_rgfMask)
    return;
  document_->buffer()->SetStyle(range_->GetStart(), range_->GetEnd(),
                                &style_attrs);
}

}  // namespace
