#!/usr/bin/env python
# Copyright (c) 2014 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import json, os, sys

BIDI_CLASS_MAP = {}
CATEGORY_MAP = {}

def build(input_file):
  input = open(input_file, 'rt')
  ucd = range(0x10000)
  for line in input:
    fields = line.split(';')
    code = int(fields[0], 16);
    if (code > 0xFFFF):
      break;
    category = fields[2]
    bidi_class = fields[4]
    if (not CATEGORY_MAP.has_key(category)):
      assert not BIDI_CLASS_MAP.has_key(category)
      CATEGORY_MAP[category] = len(CATEGORY_MAP)
    if (not BIDI_CLASS_MAP.has_key(bidi_class)):
      assert not CATEGORY_MAP.has_key(bidi_class)
      BIDI_CLASS_MAP[bidi_class] = len(BIDI_CLASS_MAP)
    ucd[code] = {
      'code': code,
      'name': fields[1],
      'gc': CATEGORY_MAP[category],
      'bc': BIDI_CLASS_MAP[bidi_class],
    }
  for code in range(0x10000):
    if (ucd[code] == code):
      ucd[code] = {
        'code': code,
        'name': "",
        'gc': 0,
        'bc': 0,
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

#include "base/basictypes.h"
#pragma warning(push)
#pragma warning(disable: 4100 4127 4530)
#include "gin/converter.h"
#pragma warning(pop)

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
    output.write('  { %(gc)2d, %(bc)2d, "%(name)s" }, // %(code)04X\n' %
        ucd[code])
  output.write("""\
};

v8::Handle<v8::Array> CreateUcd(v8::Isolate* isolate) {
  v8::EscapableHandleScope handle_scope(isolate);
  auto ucd = v8::Array::New(isolate, 0x10000);

  // Generic Category
""")

  for name in CATEGORY_MAP:
    output.write("""\
  ucd->Set(gin::StringToV8(isolate, "%(name)s"), v8::Integer::New(isolate, %(value)d));
""" % {'name': name, 'value': CATEGORY_MAP[name]})

  output.write("""\

  // Bidi Class
""")
  for name in BIDI_CLASS_MAP:
    output.write("""\
  ucd->Set(gin::StringToV8(isolate, "%(name)s"), v8::Integer::New(isolate, %(value)d));
""" % {'name': name, 'value': BIDI_CLASS_MAP[name]})

  output.write("""\

  // Character data
  auto name = gin::StringToV8(isolate, "name");
  auto category = gin::StringToV8(isolate, "gc");
  auto bidi_class = gin::StringToV8(isolate, "bc");
  for (auto code = 0; code <= 0xFFFF; ++code) {
    auto const entry = &kUnicodeData[code];
    auto data = v8::Object::New(isolate);
    data->ForceSet(name, gin::StringToV8(isolate, entry->name));
    data->ForceSet(category, v8::Integer::New(isolate, entry->category));
    data->ForceSet(bidi_class, v8::Integer::New(isolate, entry->bidi_class));
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
""")

  output.close()

def main():
  output_file = sys.argv[1];
  input_file = sys.argv[2];
  ucd = build(input_file)
  index = 0
  emitJs(output_file, ucd)

if __name__ == '__main__':
  main()
