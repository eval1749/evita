// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#pragma warning(disable: 4100 4127 4251 4530)
#include "base/basictypes.h"
#include "base/i18n/icu_util.h"
#include "base/logging.h"
#include "gin/converter.h"
#include "third_party/icu/source/common/unicode/uchar.h"
#include "third_party/icu/source/common/unicode/utypes.h"

namespace dom {
namespace internal {

namespace {
const char* kBidiClassNames[] = {
  "L", "R", "EN", "ES", "ET", "AN", "CS", "B", "S", "WS", "ON", "LRE", "LRO",
  "AL", "RLE", "RLO", "PDF", "NSM", "BN",
};

const char* kCategoryNames[] = {
  "Cn", "Lu", "Ll", "Lt", "Lm", "Lo", "Mn", "Me", "Mc", "Nd", "Nl", "No",
  "Zs", "Zl", "Zp", "Cc", "Cf", "Co", "Cs", "Pd", "Ps", "Pe", "Pc", "Po",
  "Sm", "Sc", "Sk", "So", "Pi", "Pf",
};

v8::Handle<v8::Array> CreateUcd(v8::Isolate* isolate) {
  v8::EscapableHandleScope handle_scope(isolate);
  auto ucd = v8::Array::New(isolate, 0x10000);

  // Bidi class name
  for (auto i = 0; i < arraysize(kBidiClassNames); ++i) {
    ucd->Set(gin::StringToV8(isolate, kBidiClassNames[i]),
             v8::Integer::New(isolate, i));
  }

  // Category name
  for (auto i = 0; i < arraysize(kCategoryNames); ++i) {
    ucd->Set(gin::StringToV8(isolate, kCategoryNames[i]),
             v8::Integer::New(isolate, i));
  }

  auto name = gin::StringToV8(isolate, "name");
  auto category = gin::StringToV8(isolate, "gc");
  auto bidi_class = gin::StringToV8(isolate, "bc");
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
    data->ForceSet(bidi_class, v8::Integer::New(isolate, bidi_class_index));

    // gc
    auto const category_index = u_charType(code);
    CHECK(category_index < arraysize(kCategoryNames));
    data->ForceSet(category, v8::Integer::New(isolate, category_index));

    ucd->Set(code, data);
  }
  return handle_scope.Escape(ucd);
}

// Note: Creating UCD object takes a few second in debug build. To make
// test faster, we cache UCD object.
class Ucd {
  private: std::unique_ptr<v8::UniquePersistent<v8::Array>> ucd_;

  public: Ucd(v8::Isolate* isolate)
    : ucd_(new v8::UniquePersistent<v8::Array>()) {
    ucd_->Reset(isolate, CreateUcd(isolate));
  }
  public: ~Ucd() = default;

  public: v8::Handle<v8::Array> Get(v8::Isolate* isolate) {
    return v8::Local<v8::Array>::New(isolate, *ucd_);
  }
};

}  // namespace

v8::Handle<v8::Array> GetUcdObject(v8::Isolate* isolate) {
  CR_DEFINE_STATIC_LOCAL(Ucd, static_ucd, (isolate));
  return static_ucd.Get(isolate);
}

}  // namespace internal
}  // namespace dom
