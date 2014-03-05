// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/os/file_handle.h"

#include "base/memory/ref_counted.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/script_callback.h"

namespace dom {

namespace {
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
    auto const js_handle = error_code ? v8::Null::New(isolate) :
        (new FileHandle(handle))->GetWrapper(isolate);
    runner_->Call(function, js_handle, v8::Integer(isolate, error_code));
  }
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

  private: static void OpenFile(const base::string16& file_name,
                                const base::string16& mode,
                                v8::Handle<v8::Function> callback) {
                    v8::Handle<v8::Function> callback) {
  auto const runner = ScriptController::instance()->runner();
  auto const file_io_callback = make_scoped_refptr(
      new OpenFileCallback(runner, callback));
  ScriptController::instance()->io_delegate()->OpenFile(file_name, mode,
      base::Bind(&OpenFileCallback::Run, file_io_callback));
  }

  // v8_glue::WrapperInfo
  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto templ = v8_glue::CreateConstructorTemplate(isolate,
        &FileHandleClass::NewEditor);
    return v8_glue::FunctionTemplateBuilder(isolate, templ)
      .SetMethod("openFile_", &FileHandleClass::OpenFile)
      .Build();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetMethod("close", &FileHandle::Close)
        .SetMethod("readTo", &FileHandle::ReadTo)
        .SetMethod("writeFrom", &FileHandle::WriteFrom);
  }

  DISALLOW_COPY_AND_ASSIGN(FileHandleClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FileHandle::HandlingFormEventScope
//
FileHandle::HandlingFormEventScope::HandlingFormEventScope(
    FileHandle* control) : control_(control) {
  DCHECK(!control_->handling_form_event_);
  control_->handling_form_event_ = true;
}

FileHandle::HandlingFormEventScope::~HandlingFormEventScope() {
  DCHECK(control_->handling_form_event_);
  control_->handling_form_event_ = false;
}

//////////////////////////////////////////////////////////////////////
//
// FileHandle
//
DEFINE_SCRIPTABLE_OBJECT(FileHandle, FileHandleClass);

FileHandle::FileHandle(domapi::IoHandle handle)
    : closed_(false), handle_(handle) {
}

FileHandle::~FileHandle() {
  Close();
}

void FileHandle::Close() {
  if (closed_)
    return;
  closed_ = true;
  ScriptController::instance()->io_handler()->CloseFile(handle_);
}

}  // namespace dom
