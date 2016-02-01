// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/os/file.h"

#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "evita/dom/bindings/v8_glue_MoveFileOptions.h"
#include "evita/dom/converter.h"
#include "evita/dom/os/file_status.h"
#include "evita/dom/os/io_error.h"
#include "evita/dom/promise_resolver.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/script_host.h"
#include "evita/v8_glue/runner.h"
#include "gin/array_buffer.h"

namespace dom {
base::string16 V8ToString(v8::Local<v8::Value> value);
}

namespace gin {
v8::Local<v8::Value> Converter<domapi::FileId>::ToV8(
    v8::Isolate* isolate,
    domapi::FileId context_id) {
  return gin::ConvertToV8(isolate, new dom::File(context_id));
}
}  // namespace gin

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// File
//
File::File(domapi::IoContextId context_id) : ScriptableBase(context_id) {}

File::~File() {}

v8::Local<v8::Promise> File::MakeTempFileName(const base::string16& dir_name,
                                              const base::string16& prefix) {
  return PromiseResolver::Call(
      FROM_HERE,
      base::Bind(&domapi::IoDelegate::MakeTempFileName,
                 base::Unretained(ScriptHost::instance()->io_delegate()),
                 dir_name, prefix));
}

v8::Local<v8::Promise> File::Move(const base::string16& src_path,
                                  const base::string16& dst_path,
                                  const MoveFileOptions& options) {
  domapi::MoveFileOptions api_options;
  api_options.no_overwrite = options.no_overwrite();
  return PromiseResolver::Call(
      FROM_HERE,
      base::Bind(&domapi::IoDelegate::MoveFile,
                 base::Unretained(ScriptHost::instance()->io_delegate()),
                 src_path, dst_path, api_options));
}

v8::Local<v8::Promise> File::Move(const base::string16& src_path,
                                  const base::string16& dst_path) {
  return Move(src_path, dst_path, MoveFileOptions());
}

v8::Local<v8::Promise> File::Open(const base::string16& file_name,
                                  const base::string16& mode) {
  return PromiseResolver::Call(
      FROM_HERE,
      base::Bind(&domapi::IoDelegate::OpenFile,
                 base::Unretained(ScriptHost::instance()->io_delegate()),
                 file_name, mode));
}

v8::Local<v8::Promise> File::Open(const base::string16& file_name) {
  return Open(file_name, base::string16());
}

v8::Local<v8::Promise> File::Remove(const base::string16& file_name) {
  return PromiseResolver::Call(
      FROM_HERE,
      base::Bind(&domapi::IoDelegate::RemoveFile,
                 base::Unretained(ScriptHost::instance()->io_delegate()),
                 file_name));
}

v8::Local<v8::Promise> File::Stat(const base::string16& file_name) {
  return PromiseResolver::Call(
      FROM_HERE,
      base::Bind(&domapi::IoDelegate::QueryFileStatus,
                 base::Unretained(ScriptHost::instance()->io_delegate()),
                 file_name));
}

}  // namespace dom
