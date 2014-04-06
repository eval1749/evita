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
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/runner.h"
#include "gin/array_buffer.h"
#include "v8_strings.h"

namespace gin {
template<>
struct Converter<domapi::FileId> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      domapi::FileId context_id) {
    return gin::ConvertToV8(isolate, new dom::os::File(context_id));
  }
};

template<>
struct Converter<domapi::FileStatus> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
        const domapi::FileStatus& data) {
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
};
}  // namespace gin

namespace dom {
namespace os {

namespace {

//////////////////////////////////////////////////////////////////////
//
// FileClass
//
class FileClass :
  public v8_glue::DerivedWrapperInfo<File, AbstractFile> {

  public: FileClass(const char* name)
      : BaseClass(name) {
  }
  public: ~FileClass() = default;

  private: static File* NewFile() {
    ScriptHost::instance()->ThrowError(
        "Cannot create an instance of File.");
    return nullptr;
  }

  private: static v8::Handle<v8::Promise> OpenFile(
      const base::string16& file_name,
      v8_glue::Optional<base::string16> opt_mode);
  private: static v8::Handle<v8::Promise> QueryFileStatus(
      const base::string16& filename);

  // v8_glue::WrapperInfo
  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto const templ = v8_glue::CreateConstructorTemplate(isolate,
        &FileClass::NewFile);
  return v8_glue::FunctionTemplateBuilder(isolate, templ)
      .SetMethod("makeTempFileName", &File::MakeTempFileName)
      .SetMethod("open", &FileClass::OpenFile)
      .SetMethod("stat", &FileClass::QueryFileStatus)
      .Build();
  }

  DISALLOW_COPY_AND_ASSIGN(FileClass);
};

v8::Handle<v8::Promise> FileClass::OpenFile(const base::string16& file_name,
    v8_glue::Optional<base::string16> opt_mode) {
  return PromiseResolver::Call(base::Bind(
      &domapi::IoDelegate::OpenFile,
      base::Unretained(ScriptHost::instance()->io_delegate()),
      file_name, opt_mode.is_supplied ? opt_mode.value : base::string16()));
}

v8::Handle<v8::Promise> FileClass::QueryFileStatus(
    const base::string16& file_name) {
  return PromiseResolver::Call(base::Bind(
      &domapi::IoDelegate::QueryFileStatus,
      base::Unretained(ScriptHost::instance()->io_delegate()),
      file_name));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// File
//
DEFINE_SCRIPTABLE_OBJECT(File, FileClass);

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

}  // namespace os
}  // namespace dom
