// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/os/file.h"

#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "evita/dom/converter.h"
#include "evita/dom/promise_resolver.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/public/io_error.h"
#include "evita/dom/script_host.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/script_callback.h"
#include "gin/array_buffer.h"
#include "v8_strings.h"

namespace gin {
v8::Handle<v8::Value> Converter<domapi::IoError>::ToV8(
    v8::Isolate* isolate, const domapi::IoError& error) {
  auto const runner = v8_glue::Runner::current_runner(isolate);
  auto const os_file_error_ctor = runner->global()->
      Get(dom::v8Strings::Os.Get(isolate))->ToObject()->
      Get(dom::v8Strings::File.Get(isolate))->ToObject()->
      Get(dom::v8Strings::Error.Get(isolate));
  return runner->CallAsConstructor(os_file_error_ctor,
      v8::Integer::New(isolate, error.error_code));
}
}  // namespace gin

namespace dom {
namespace os {

namespace {

//////////////////////////////////////////////////////////////////////
//
// AbstractFileClass
//
class AbstractFileClass : public v8_glue::WrapperInfo {
  private: typedef v8_glue::WrapperInfo BaseClass;

  public: AbstractFileClass(const char* name)
      : BaseClass(name) {
  }
  public: ~AbstractFileClass() = default;

  private: static File* NewFile() {
    ScriptHost::instance()->ThrowError(
        "Cannot create an instance of File.");
    return nullptr;
  }

  // v8_glue::WrapperInfo
  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &AbstractFileClass::NewFile);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetMethod("close", &AbstractFile::Close)
        .SetMethod("read", &AbstractFile::Read)
        .SetMethod("write", &AbstractFile::Write);
  }

  DISALLOW_COPY_AND_ASSIGN(AbstractFileClass);
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// File
//
DEFINE_SCRIPTABLE_OBJECT(AbstractFile, AbstractFileClass);

AbstractFile::AbstractFile(domapi::IoContextId context_id)
    : context_id_(context_id) {
}

AbstractFile::~AbstractFile() {
  Close();
}

v8::Handle<v8::Promise> AbstractFile::Close() {
  return PromiseResolver::Call(base::Bind(
      &domapi::IoDelegate::CloseFile,
      base::Unretained(ScriptHost::instance()->io_delegate()),
      context_id_));
}

v8::Handle<v8::Promise> AbstractFile::Read(
    const gin::ArrayBufferView& array_buffer_view) {
  return PromiseResolver::Call(base::Bind(
      &domapi::IoDelegate::ReadFile,
      base::Unretained(ScriptHost::instance()->io_delegate()),
      context_id_, array_buffer_view.bytes(), array_buffer_view.num_bytes()));
}

v8::Handle<v8::Promise> AbstractFile::Write(
    const gin::ArrayBufferView& array_buffer_view) {
  return PromiseResolver::Call(base::Bind(
      &domapi::IoDelegate::WriteFile,
      base::Unretained(ScriptHost::instance()->io_delegate()),
      context_id_, array_buffer_view.bytes(), array_buffer_view.num_bytes()));
}

}  // namespace os
}  // namespace dom
