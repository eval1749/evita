// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

// L4 C4100: 'identifier' : unreferenced formal parameter
// L4 C4251: 'identifier' : class 'type' needs to have dll-interface to be
// used by clients of class 'type2'
// L4 C4365: 'action' : conversion from 'type_1' to 'type_2', signed/unsigned
// mismatch
// L1 C4530: C++ exception handler used, but unwind semantics are not enabled.
// Specify /EHsc
#pragma warning(disable: 4127 4251 4530)
#include "base/basictypes.h"
#include "base/i18n/icu_util.h"
// L4 C4127: conditional expression is constant
#pragma warning(push)
#pragma warning(disable: 4127)
#include "base/logging.h"
#pragma warning(pop)
#include "evita/v8_glue/converter.h"
#include "third_party/icu/source/common/unicode/uchar.h"
#include "third_party/icu/source/common/unicode/utypes.h"

namespace dom {
namespace internal {

namespace {
// Note: The order of |kBidiClassNames| must be matched to |UCharDirection|.
const char* kBidiClassNames[] = {
  "L", "R", "EN", "ES", "ET", "AN", "CS", "B", "S", "WS", "ON", "LRE", "LRO",
  "AL", "RLE", "RLO", "PDF", "NSM", "BN",
};

// Note: The order of |kCategoryNames| must be matched to |UCharCategory|.
const char* kCategoryNames[] = {
  "Cn", "Lu", "Ll", "Lt", "Lm", "Lo", "Mn", "Me", "Mc", "Nd", "Nl", "No",
  "Zs", "Zl", "Zp", "Cc", "Cf", "Co", "Cs", "Pd", "Ps", "Pe", "Pc", "Po",
  "Sm", "Sc", "Sk", "So", "Pi", "Pf",
};

#define DCHECK_EQ_CHAR_1(vector, index, name) \
  DCHECK((vector)[index][0] == (name)[0] && \
         (vector)[index][1] == (name)[1])

#define DCHECK_EQ_CHAR_2(vector, index, name) \
  DCHECK((vector)[index][0] == (name)[0] && \
         (vector)[index][1] == (name)[1] && \
         (vector)[index][2] == (name)[2])

#define DCHECK_EQ_CHAR_3(vector, index, name) \
  DCHECK((vector)[index][0] == (name)[0] && \
         (vector)[index][1] == (name)[1] && \
         (vector)[index][2] == (name)[2] && \
         (vector)[index][3] == (name)[3])

// Unicode.BIDI_CLASS_SHORT_NAMES : Array.<string>
// Unicode.GENERAL_CATEGORY_SHOT_NAMES : Array.<string>
// Unicode.Bidi : enum
// Unicode.Category : enum
// Uicode.UCD : Array.<{bidi: Unicode.Bidi, category: Unicode.Category}>
v8::Handle<v8::Object> CreateUnicode(v8::Isolate* isolate) {
  DCHECK_EQ_CHAR_1(kBidiClassNames, U_LEFT_TO_RIGHT, "L");
  DCHECK_EQ_CHAR_1(kBidiClassNames, U_RIGHT_TO_LEFT, "R");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_EUROPEAN_NUMBER, "EN");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_EUROPEAN_NUMBER_SEPARATOR, "ES");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_EUROPEAN_NUMBER_TERMINATOR, "ET");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_ARABIC_NUMBER, "AN");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_COMMON_NUMBER_SEPARATOR, "CS");
  DCHECK_EQ_CHAR_1(kBidiClassNames, U_BLOCK_SEPARATOR, "B");
  DCHECK_EQ_CHAR_1(kBidiClassNames, U_SEGMENT_SEPARATOR, "S");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_WHITE_SPACE_NEUTRAL, "WS");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_OTHER_NEUTRAL, "ON");
  DCHECK_EQ_CHAR_3(kBidiClassNames, U_LEFT_TO_RIGHT_EMBEDDING, "LRE");
  DCHECK_EQ_CHAR_3(kBidiClassNames, U_LEFT_TO_RIGHT_OVERRIDE, "LRO");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_RIGHT_TO_LEFT_ARABIC, "AL");
  DCHECK_EQ_CHAR_3(kBidiClassNames, U_RIGHT_TO_LEFT_EMBEDDING, "RLE");
  DCHECK_EQ_CHAR_3(kBidiClassNames, U_RIGHT_TO_LEFT_OVERRIDE, "RLO");
  DCHECK_EQ_CHAR_3(kBidiClassNames, U_POP_DIRECTIONAL_FORMAT, "PDF");
  DCHECK_EQ_CHAR_3(kBidiClassNames, U_DIR_NON_SPACING_MARK, "NSM");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_BOUNDARY_NEUTRAL, "BN");

  DCHECK_EQ_CHAR_2(kCategoryNames, U_UNASSIGNED, "Cn");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_UPPERCASE_LETTER, "Lu");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_LOWERCASE_LETTER, "Ll");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_TITLECASE_LETTER, "Lt");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_MODIFIER_LETTER, "Lm");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_OTHER_LETTER, "Lo");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_NON_SPACING_MARK, "Mn");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_ENCLOSING_MARK, "Me");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_COMBINING_SPACING_MARK, "Mc");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_DECIMAL_DIGIT_NUMBER, "Nd");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_LETTER_NUMBER, "Nl");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_OTHER_NUMBER, "No");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_SPACE_SEPARATOR, "Zs");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_LINE_SEPARATOR, "Zl");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_PARAGRAPH_SEPARATOR, "Zp");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_CONTROL_CHAR, "Cc");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_FORMAT_CHAR, "Cf");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_PRIVATE_USE_CHAR, "Co");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_SURROGATE, "Cs");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_DASH_PUNCTUATION, "Pd");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_START_PUNCTUATION, "Ps");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_END_PUNCTUATION, "Pe");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_CONNECTOR_PUNCTUATION, "Pc");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_OTHER_PUNCTUATION, "Po");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_MATH_SYMBOL, "Sm");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_CURRENCY_SYMBOL, "Sc");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_MODIFIER_SYMBOL, "Sk");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_OTHER_SYMBOL, "So");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_INITIAL_PUNCTUATION, "Pi");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_FINAL_PUNCTUATION, "Pf");

  v8::EscapableHandleScope handle_scope(isolate);
  auto unicode = v8::Object::New(isolate);

  // Bidi class list
  auto bidi_names = v8::Array::New(isolate, arraysize(kBidiClassNames));
  auto bidi_object = v8::Object::New(isolate);
  for (auto i = 0; i < arraysize(kBidiClassNames); ++i) {
    auto name = gin::StringToV8(isolate, kBidiClassNames[i]);
    bidi_object->Set(name, name);
    bidi_names->Set(static_cast<size_t>(i), name);
  }
  unicode->Set(gin::StringToV8(isolate, "Bidi"), bidi_object);
  unicode->Set(gin::StringToV8(isolate, "BIDI_CLASS_SHORT_NAMES"), bidi_names);

  // Category name
  auto category_names = v8::Array::New(isolate, arraysize(kCategoryNames));
  auto category_object = v8::Object::New(isolate);
  for (auto i = 0; i < arraysize(kCategoryNames); ++i) {
    auto name = gin::StringToV8(isolate, kCategoryNames[i]);
    category_object->Set(name, name);
    category_names->Set(static_cast<size_t>(i), name);
  }
  unicode->Set(gin::StringToV8(isolate, "Category"), category_object);
  unicode->Set(gin::StringToV8(isolate, "CATEGORY_SHORT_NAMES"),
               category_names);

  auto ucd = v8::Array::New(isolate, 0x10000);
  unicode->Set(gin::StringToV8(isolate, "UCD"), ucd);
  auto name = gin::StringToV8(isolate, "name");
  auto category = gin::StringToV8(isolate, "category");
  auto bidi_class = gin::StringToV8(isolate, "bidi");
  for (auto code = 0; code <= 0xFFFF; ++code) {
    auto data = v8::Object::New(isolate);

#if 0
    // Note: Because of Chromium icudata doesn't contain unames.icu. So, we
    // can't have character names.
    // name
    auto errorCode = U_ZERO_ERROR;
    char name_buffer[100];
    u_charName(code, U_UNICODE_CHAR_NAME, name_buffer, sizeof(name_buffer),
               &errorCode);
    CHECK_EQ(U_ZERO_ERROR, errorCode);
    data->ForceSet(name, gin::StringToV8(isolate, name_buffer));
#endif

    // bc
    auto const bidi_class_index = u_charDirection(code);
    CHECK(bidi_class_index < arraysize(kBidiClassNames));
    data->ForceSet(bidi_class, bidi_names->Get(bidi_class_index));

    // gc
    auto const category_index = u_charType(code);
    CHECK(category_index < arraysize(kCategoryNames));
    data->ForceSet(category, category_names->Get(
        static_cast<size_t>(category_index)));

    ucd->Set(static_cast<size_t>(code), data);
  }
  return handle_scope.Escape(unicode);
}

// Note: Creating UCD object takes a few second in debug build. To make
// test faster, we cache UCD object.
class Unicode {
  private: std::unique_ptr<v8::UniquePersistent<v8::Object>> unicode_;

  public: Unicode(v8::Isolate* isolate)
    : unicode_(new v8::UniquePersistent<v8::Object>()) {
    unicode_->Reset(isolate, CreateUnicode(isolate));
  }
  public: ~Unicode() = default;

  public: v8::Handle<v8::Object> Get(v8::Isolate* isolate) {
    return v8::Local<v8::Object>::New(isolate, *unicode_);
  }

  DISALLOW_COPY_AND_ASSIGN(Unicode);
};

}  // namespace

v8::Handle<v8::Object> GetUnicodeObject(v8::Isolate* isolate) {
  CR_DEFINE_STATIC_LOCAL(Unicode, static_unicode, (isolate));
  return static_unicode.Get(isolate);
}

}  // namespace internal
}  // namespace dom
