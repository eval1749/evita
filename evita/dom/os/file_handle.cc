// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/os/file_handle.h"

#include "base/memory/ref_counted.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/script_callback.h"
#include "gin/array_buffer.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// FileIoCallback
//
class FileIoCallback : public base::RefCounted<FileIoCallback> {
  private: v8_glue::ScopedPersistent<v8::Function> function_;
  private: base::WeakPtr<v8_glue::Runner> runner_;

  public: FileIoCallback(v8_glue::Runner* runner,
                          v8::Handle<v8::Function> function)
    : function_(runner->isolate(), function), runner_(runner->GetWeakPtr()) {
  }

  public: void Run(int num_transfered, int error_code) {
    if (!runner_)
      return;
    v8_glue::Runner::Scope runner_scope(runner_.get());
    auto const isolate = runner_->isolate();
    auto const function = function_.NewLocal(isolate);
    runner_->Call(function, v8::Undefined(isolate),
                  v8::Integer::New(isolate, num_transfered),
                  v8::Integer::New(isolate, error_code));
  }

  DISALLOW_COPY_AND_ASSIGN(FileIoCallback);
};

//////////////////////////////////////////////////////////////////////
//
// FileHandleClass
//
class FileHandleClass : public v8_glue::WrapperInfo {
  private: typedef v8_glue::WrapperInfo BaseClass;

  public: FileHandleClass(const char* name)
      : BaseClass(name) {
  }
  public: ~FileHandleClass() = default;

  private: static FileHandle* NewFileHandle() {
    ScriptController::instance()->ThrowError(
        "Cannot create an instance of FileHandle.");
    return nullptr;
  }

  // v8_glue::WrapperInfo
  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &FileHandleClass::NewFileHandle);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetMethod("close", &FileHandle::Close)
        .SetMethod("read_", &FileHandle::Read)
        .SetMethod("write_", &FileHandle::Write);
  }

  DISALLOW_COPY_AND_ASSIGN(FileHandleClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FileHandle
//
DEFINE_SCRIPTABLE_OBJECT(FileHandle, FileHandleClass);

FileHandle::FileHandle(domapi::IoHandle* handle)
    : closed_(false), handle_(handle) {
}

FileHandle::~FileHandle() {
  Close();
}

void FileHandle::Close() {
  if (closed_)
    return;
  closed_ = true;
  ScriptController::instance()->io_delegate()->CloseFile(handle_);
}

void FileHandle::Read(const gin::ArrayBufferView& array_buffer_view,
                      v8::Handle<v8::Function> callback) {
  auto const runner = ScriptController::instance()->runner();
  auto const file_io_callback = make_scoped_refptr(
      new FileIoCallback(runner, callback));
  ScriptController::instance()->io_delegate()->ReadFile(
      handle_, array_buffer_view.bytes(), array_buffer_view.num_bytes(),
      base::Bind(&FileIoCallback::Run, file_io_callback));
}

void FileHandle::Write(const gin::ArrayBufferView& array_buffer_view,
                       v8::Handle<v8::Function> callback) {
  auto const runner = ScriptController::instance()->runner();
  auto const file_io_callback = make_scoped_refptr(
      new FileIoCallback(runner, callback));
  ScriptController::instance()->io_delegate()->WriteFile(
      handle_, array_buffer_view.bytes(), array_buffer_view.num_bytes(),
      base::Bind(&FileIoCallback::Run, file_io_callback));
}

}  // namespace dom
