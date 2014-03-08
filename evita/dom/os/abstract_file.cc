// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/os/file.h"

#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "evita/dom/converter.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/promise_callback.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/script_callback.h"
#include "gin/array_buffer.h"
#include "v8_strings.h"

namespace gin {
v8::Handle<v8::Value> Converter<dom::os::FileError>::ToV8(
    v8::Isolate* isolate, const dom::os::FileError& error) {
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
// CloseFileCallback
//
class CloseFileCallback : public v8_glue::PromiseCallback {
  public: CloseFileCallback(v8_glue::Runner* runner)
    : v8_glue::PromiseCallback(runner) {
  }
  public: ~CloseFileCallback() = default;

  public: void Run(int error_code) {
    if (error_code) {
      Reject(FileError(error_code));
      return;
    }
    Resolve(0);
  }

  DISALLOW_COPY_AND_ASSIGN(CloseFileCallback);
};

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
    ScriptController::instance()->ThrowError(
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

//////////////////////////////////////////////////////////////////////
//
// FileIoCallback
//
class FileIoCallback : public v8_glue::PromiseCallback {
  public: FileIoCallback(v8_glue::Runner* runner)
    : v8_glue::PromiseCallback(runner) {
  }
  public: ~FileIoCallback() = default;

  public: void Run(int num_transfered, int error_code) {
    if (error_code) {
      Reject(FileError(error_code));
      return;
    }
    Resolve(num_transfered);
  }

  DISALLOW_COPY_AND_ASSIGN(FileIoCallback);
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

v8::Handle<v8::Object> AbstractFile::Close() {
  auto const runner = ScriptController::instance()->runner();
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);
  auto const callback = make_scoped_refptr(new CloseFileCallback(runner));
  ScriptController::instance()->io_delegate()->CloseFile(context_id_,
      base::Bind(&CloseFileCallback::Run, callback));
  return runner_scope.Escape(callback->GetPromise(runner->isolate()));
}

v8::Handle<v8::Object> AbstractFile::Read(
    const gin::ArrayBufferView& array_buffer_view) {
  auto const runner = ScriptController::instance()->runner();
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);
  auto const file_io_callback = make_scoped_refptr(
      new FileIoCallback(runner));
  ScriptController::instance()->io_delegate()->ReadFile(
      context_id_, array_buffer_view.bytes(), array_buffer_view.num_bytes(),
      base::Bind(&FileIoCallback::Run, file_io_callback));
  return runner_scope.Escape(file_io_callback->GetPromise(runner->isolate()));
}

v8::Handle<v8::Object> AbstractFile::Write(
    const gin::ArrayBufferView& array_buffer_view) {
  auto const runner = ScriptController::instance()->runner();
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);
  auto const file_io_callback = make_scoped_refptr(
      new FileIoCallback(runner));
  ScriptController::instance()->io_delegate()->WriteFile(
      context_id_, array_buffer_view.bytes(), array_buffer_view.num_bytes(),
      base::Bind(&FileIoCallback::Run, file_io_callback));
  return runner_scope.Escape(file_io_callback->GetPromise(runner->isolate()));
}

}  // namespace os
}  // namespace dom
