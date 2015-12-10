// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/os/file_status.h"

#include "evita/dom/converter.h"
#include "evita/v8_glue/runner.h"
#include "v8_strings.h"  // NOLINT(build/include)

namespace gin {
v8::Handle<v8::Value> Converter<domapi::FileStatus>::ToV8(
    v8::Isolate* isolate,
    const domapi::FileStatus& data) {
  auto const runner = v8_glue::Runner::current_runner(isolate);
  auto const os_file_info_ctor = runner->global()
                                     ->Get(dom::v8Strings::Os.Get(isolate))
                                     ->ToObject()
                                     ->Get(dom::v8Strings::File.Get(isolate))
                                     ->ToObject()
                                     ->Get(dom::v8Strings::Info.Get(isolate));
  auto const js_data = runner->CallAsConstructor(os_file_info_ctor)->ToObject();
  js_data->Set(dom::v8Strings::isDir.Get(isolate),
               gin::ConvertToV8(isolate, data.is_directory));
  js_data->Set(dom::v8Strings::isSymLink.Get(isolate),
               gin::ConvertToV8(isolate, data.is_symlink));
  js_data->Set(dom::v8Strings::lastModificationDate.Get(isolate),
               gin::ConvertToV8(isolate, data.last_write_time));
  js_data->Set(dom::v8Strings::name.Get(isolate),
               gin::ConvertToV8(isolate, data.name));
  js_data->Set(dom::v8Strings::readonly.Get(isolate),
               gin::ConvertToV8(isolate, data.readonly));
  js_data->Set(dom::v8Strings::size.Get(isolate),
               gin::ConvertToV8(isolate, data.file_size));
  return js_data;
}
}  // namespace gin
