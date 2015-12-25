#!/usr/bin/env python
# Copyright (c) 2014 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import json
import os
import sys

BIDI_CLASS_MAP = {}
BIDI_CLASS_NAMES = []
CATEGORY_MAP = {}
CATEGORY_NAMES = []


def build(input_file):
    input_file = open(input_file, 'rt')
    ucd = range(0x10000)
    for line in input_file:
        fields = line.split(';')
        code = int(fields[0], 16)
        if (code > 0xFFFF):
            break
        category = fields[2]
        bidi_class = fields[4]
        if (category not in CATEGORY_MAP):
            assert category not in BIDI_CLASS_MAP
            CATEGORY_MAP[category] = len(CATEGORY_MAP)
            CATEGORY_NAMES.append(category)
        if (bidi_class not in BIDI_CLASS_MAP):
            assert bidi_class not in CATEGORY_MAP
            BIDI_CLASS_MAP[bidi_class] = len(BIDI_CLASS_MAP)
            BIDI_CLASS_NAMES.append(bidi_class)
        ucd[code] = {
            'code': code,
            'name': fields[1],
            'category': CATEGORY_MAP[category],
            'bidi': BIDI_CLASS_MAP[bidi_class],
        }
    for code in range(0x10000):
        if (ucd[code] == code):
            ucd[code] = {
                'code': code,
                'name': "",
                'category': 0,
                'bidi': 0,
            }
    input.close()
    return ucd


def emitJs(output_file, ucd):
    output = open(output_file, 'w')
    output.write("""\
// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "base/macros.h"
#include "gin/converter.h"

namespace dom {
namespace internal {
namespace {
struct Entry {
  int category;
  int bidi_class;
  const char* name;
};

const Entry kUnicodeData[] = {
""")
    for code in range(0x10000):
        output.write(
            '  { %(category)2d, %(bidi)2d, "%(name)s" }, // %(code)04X\n' %
            ucd[code])
    output.write("""\
};

const char* kBidiClassNames[] = {
""")

    index = 0
    for name in BIDI_CLASS_NAMES:
        output.write("""  "%(name)s, // %(index)d"\n""" %
                     {'index': index, 'name': name})
        index += 1
    output.write("""\
};

const char* kCategoryNames[] = {
""")
    index = 0
    for name in CATEGORY_NAMES:
        output.write("""  "%(name)s", // %(index)d\n""" %
                     {'index': index, 'name': name})
        index += 1
    output.write("""\
};

// Unicode.BIDI_CLASS_SHORT_NAMES : Array.<string>
// Unicode.GENERAL_CATEGORY_SHOT_NAMES : Array.<string>
// Unicode.Bidi : enum
// Unicode.Category : enum
// Uicode.UCD : Array.<{bidi: Unicode.Bidi, category: Unicode.Category}>
v8::Handle<v8::Object> CreateUnicode(v8::Isolate* isolate) {
  v8::EscapableHandleScope handle_scope(isolate);
  auto unicode = v8::Object::New(isolate);

  // Bidi class list
  auto bidi_names = v8::Array::New(isolate, arraysize(kBidiClassNames));
  auto bidi_object = v8::Object::New(isolate);
  for (auto i = 0; i < arraysize(kBidiClassNames); ++i) {
    auto name = gin::StringToV8(isolate, kBidiClassNames[i]);
    bidi_object->Set(name, name);
    bidi_names->Set(i, name);
  }
  unicode->Set(gin::StringToV8(isolate, "Bidi"), bidi_object);
  unicode->Set(gin::StringToV8(isolate, "BIDI_CLASS_SHORT_NAMES"), bidi_names);

  // Category name
  auto category_names = v8::Array::New(isolate, arraysize(kCategoryNames));
  auto category_object = v8::Object::New(isolate);
  for (auto i = 0; i < arraysize(kCategoryNames); ++i) {
    auto name = gin::StringToV8(isolate, kCategoryNames[i]);
    category_object->Set(name, name);
    category_names->Set(i, name);
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
    auto entry = &kUnicodeData[code];

    data->ForceSet(name, gin::StringToV8(isolate, entry->name));

    data->ForceSet(bidi_class, gin::StringToV8(isolate,
        kBidiClassNames[entry->bidi_class]));

    data->ForceSet(category, gin::StringToV8(isolate,
        kCategoryNames[entry->category]));

    ucd->Set(code, data);
  }
  return handle_scope.Escape(unicode);
}

// Note: Creating UCD object takes a few second in debug build. To make
// test faster, we cache UCD object.
class Unicode final {
 public:
  Unicode(v8::Isolate* isolate)
    : unicode_(new v8::UniquePersistent<v8::Object>()) {
    unicode_->Reset(isolate, CreateUnicode(isolate));
  }
  ~Unicode() = default;

  v8::Handle<v8::Object> Get(v8::Isolate* isolate) {
    return v8::Local<v8::Object>::New(isolate, *unicode_);
  }

 private:
  std::unique_ptr<v8::UniquePersistent<v8::Object>> unicode_;

  DISALLOW_COPY_AND_ASSIGN(Unicode);
};

}  // namespace

v8::Handle<v8::Object> GetUnicodeObject(v8::Isolate* isolate) {
  CR_DEFINE_STATIC_LOCAL(Unicode, static_unicode, (isolate));
  return static_unicode.Get(isolate);
}

}  // namespace internal
}  // namespace dom
""")

    output.close()


def main():
    output_file = sys.argv[1]
    input_file = sys.argv[2]
    ucd = build(input_file)
    emitJs(output_file, ucd)

if __name__ == '__main__':
    main()
