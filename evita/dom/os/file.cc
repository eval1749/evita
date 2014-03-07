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
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/script_callback.h"
#include "gin/array_buffer.h"
#include "v8_strings.h"

namespace dom {
namespace os {

namespace {

//////////////////////////////////////////////////////////////////////
//
// FileClass
//
class FileClass : public v8_glue::WrapperInfo {
  private: typedef v8_glue::WrapperInfo BaseClass;

  public: FileClass(const char* name)
      : BaseClass(name) {
  }
  public: ~FileClass() = default;

  private: static File* NewFile() {
    ScriptController::instance()->ThrowError(
        "Cannot create an instance of File.");
    return nullptr;
  }

  private: static void OpenFile(const base::string16& file_name,
                                const base::string16& mode,
                                v8::Handle<v8::Function> callback);

  private: static void QueryFileStatus(const base::string16& filename,
                                        v8::Handle<v8::Function> callback);

  // v8_glue::WrapperInfo
  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto const templ = v8_glue::CreateConstructorTemplate(isolate,
        &FileClass::NewFile);
  return v8_glue::FunctionTemplateBuilder(isolate, templ)
      .SetMethod("open_", &FileClass::OpenFile)
      .SetMethod("stat_", &FileClass::QueryFileStatus)
      .Build();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetMethod("close", &File::Close)
        .SetMethod("read_", &File::Read)
        .SetMethod("write_", &File::Write);
  }

  DISALLOW_COPY_AND_ASSIGN(FileClass);
};

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
// OpenFileCallback
//
class OpenFileCallback : public base::RefCounted<OpenFileCallback> {
  private: v8_glue::ScopedPersistent<v8::Function> function_;
  private: base::WeakPtr<v8_glue::Runner> runner_;

  public: OpenFileCallback(v8_glue::Runner* runner,
                          v8::Handle<v8::Function> function)
    : function_(runner->isolate(), function), runner_(runner->GetWeakPtr()) {
  }

  public: void Run(domapi::IoHandle* handle, int error_code) {
    if (!runner_)
      return;
    v8_glue::Runner::Scope runner_scope(runner_.get());
    auto const isolate = runner_->isolate();
    auto const function = function_.NewLocal(isolate);
    if (error_code) {
      runner_->Call(function, v8::Undefined(isolate),
                    v8::Integer::New(isolate, error_code));
    } else {
      runner_->Call(function, v8::Undefined(isolate),
                    (new File(handle))->GetWrapper(isolate));
    }
  }

  DISALLOW_COPY_AND_ASSIGN(OpenFileCallback);
};

//////////////////////////////////////////////////////////////////////
//
// QueryFileStatusCallback
//
class QueryFileStatusCallback
    : public base::RefCounted<QueryFileStatusCallback> {
  private: v8_glue::ScopedPersistent<v8::Function> function_;
  private: base::WeakPtr<v8_glue::Runner> runner_;

  public: QueryFileStatusCallback(v8_glue::Runner* runner,
                                  v8::Handle<v8::Function> function)
      : function_(runner->isolate(), function), runner_(runner->GetWeakPtr()) {
  }

  public: ~QueryFileStatusCallback() = default;

  public: void Run(const domapi::QueryFileStatusCallbackData& data) {
    if (!runner_)
      return;
    v8_glue::Runner::Scope runner_scope(runner_.get());
    auto const isolate = runner_->isolate();
    auto const function = function_.NewLocal(isolate);
    auto const js_data = v8::Object::New(isolate);
    js_data->Set(v8Strings::errorCode.Get(isolate),
                 gin::ConvertToV8(isolate, data.error_code));
    js_data->Set(v8Strings::isDir.Get(isolate),
                 gin::ConvertToV8(isolate, data.is_directory));
    js_data->Set(v8Strings::isSymLink.Get(isolate),
                 gin::ConvertToV8(isolate, data.is_symlink));
    js_data->Set(v8Strings::lastModificationDate.Get(isolate),
                 gin::ConvertToV8(isolate, data.last_write_time));
    js_data->Set(v8Strings::readonly.Get(isolate),
                 gin::ConvertToV8(isolate, data.readonly));
    runner_->Call(function, v8::Undefined(isolate), js_data);
    js_data->Set(v8Strings::size.Get(isolate),
                 gin::ConvertToV8(isolate, data.file_size));
  }

  DISALLOW_COPY_AND_ASSIGN(QueryFileStatusCallback);
};

void FileClass::OpenFile(const base::string16& file_name,
                         const base::string16& mode,
                         v8::Handle<v8::Function> callback) {
  auto const runner = ScriptController::instance()->runner();
  auto const file_io_callback = make_scoped_refptr(
      new OpenFileCallback(runner, callback));
  ScriptController::instance()->io_delegate()->OpenFile(file_name, mode,
      base::Bind(&OpenFileCallback::Run, file_io_callback));
}

void FileClass::QueryFileStatus(const base::string16& filename,
                                v8::Handle<v8::Function> callback) {
  auto const runner = ScriptController::instance()->runner();
  auto const query_file_status_callback = make_scoped_refptr(
      new QueryFileStatusCallback(runner, callback));
  ScriptController::instance()->io_delegate()->QueryFileStatus(
      filename, base::Bind(&QueryFileStatusCallback::Run,
                           query_file_status_callback));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// File
//
DEFINE_SCRIPTABLE_OBJECT(File, FileClass);

File::File(domapi::IoHandle* handle)
    : closed_(false), handle_(handle) {
}

File::~File() {
  Close();
}

void File::Close() {
  if (closed_)
    return;
  closed_ = true;
  ScriptController::instance()->io_delegate()->CloseFile(handle_);
}

void File::Read(const gin::ArrayBufferView& array_buffer_view,
                      v8::Handle<v8::Function> callback) {
  auto const runner = ScriptController::instance()->runner();
  auto const file_io_callback = make_scoped_refptr(
      new FileIoCallback(runner, callback));
  ScriptController::instance()->io_delegate()->ReadFile(
      handle_, array_buffer_view.bytes(), array_buffer_view.num_bytes(),
      base::Bind(&FileIoCallback::Run, file_io_callback));
}

void File::Write(const gin::ArrayBufferView& array_buffer_view,
                       v8::Handle<v8::Function> callback) {
  auto const runner = ScriptController::instance()->runner();
  auto const file_io_callback = make_scoped_refptr(
      new FileIoCallback(runner, callback));
  ScriptController::instance()->io_delegate()->WriteFile(
      handle_, array_buffer_view.bytes(), array_buffer_view.num_bytes(),
      base::Bind(&FileIoCallback::Run, file_io_callback));
}

}  // namespace os
}  // namespace dom
