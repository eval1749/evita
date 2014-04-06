// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/os/file.h"

#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "evita/dom/converter.h"
#include "evita/dom/promise_resolver.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/script_host.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/runner.h"
#include "gin/array_buffer.h"
#include "v8_strings.h"

namespace gin {
v8::Handle<v8::Value> Converter<domapi::FileId>::ToV8(
    v8::Isolate* isolate, domapi::FileId context_id) {
  return gin::ConvertToV8(isolate, new dom::File(context_id));
}

v8::Handle<v8::Value> Converter<domapi::FileStatus>::ToV8(
    v8::Isolate* isolate, const domapi::FileStatus& data) {
  auto const runner = v8_glue::Runner::current_runner(isolate);
  auto const os_file_info_ctor = runner->global()->
      Get(dom::v8Strings::Os.Get(isolate))->ToObject()->
      Get(dom::v8Strings::File.Get(isolate))->ToObject()->
      Get(dom::v8Strings::Info.Get(isolate));
  auto const js_data = runner->CallAsConstructor(os_file_info_ctor)->
      ToObject();
  js_data->Set(dom::v8Strings::isDir.Get(isolate),
               gin::ConvertToV8(isolate, data.is_directory));
  js_data->Set(dom::v8Strings::isSymLink.Get(isolate),
               gin::ConvertToV8(isolate, data.is_symlink));
  js_data->Set(dom::v8Strings::lastModificationDate.Get(isolate),
               gin::ConvertToV8(isolate, data.last_write_time));
  js_data->Set(dom::v8Strings::readonly.Get(isolate),
               gin::ConvertToV8(isolate, data.readonly));
  js_data->Set(dom::v8Strings::size.Get(isolate),
               gin::ConvertToV8(isolate, data.file_size));
  return js_data;
}
}  // namespace gin

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// File
//
File::File(domapi::IoContextId context_id)
    : ScriptableBase(context_id) {
}

File::~File() {
}

v8::Handle<v8::Promise> File::MakeTempFileName(const base::string16& dir_name,
                                               const base::string16& prefix) {
  return PromiseResolver::Call(base::Bind(
      &domapi::IoDelegate::MakeTempFileName,
      base::Unretained(ScriptHost::instance()->io_delegate()),
      dir_name, prefix));
}

v8::Handle<v8::Promise> File::Open(const base::string16& file_name,
    v8_glue::Optional<base::string16> opt_mode) {
  return PromiseResolver::Call(base::Bind(
      &domapi::IoDelegate::OpenFile,
      base::Unretained(ScriptHost::instance()->io_delegate()),
      file_name, opt_mode.is_supplied ? opt_mode.value : base::string16()));
}

v8::Handle<v8::Promise> File::Stat(
    const base::string16& file_name) {
  return PromiseResolver::Call(base::Bind(
      &domapi::IoDelegate::QueryFileStatus,
      base::Unretained(ScriptHost::instance()->io_delegate()),
      file_name));
}

}  // namespace dom
