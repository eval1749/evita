// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE process.

#include "evita/dom/os/process.h"

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

namespace dom {
namespace os {

namespace {

//////////////////////////////////////////////////////////////////////
//
// ProcessClass
//
class ProcessClass :
  public v8_glue::DerivedWrapperInfo<Process, AbstractFile> {

  public: ProcessClass(const char* name)
      : BaseClass(name) {
  }
  public: ~ProcessClass() = default;

  private: static Process* NewProcess() {
    ScriptController::instance()->ThrowError(
        "Cannot create an instance of Process.");
    return nullptr;
  }

  private: static v8::Handle<v8::Object> OpenProcess(
      const base::string16& command_line);

  // v8_glue::WrapperInfo
  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto const templ = v8_glue::CreateConstructorTemplate(isolate,
        &ProcessClass::NewProcess);
  return v8_glue::FunctionTemplateBuilder(isolate, templ)
      .SetMethod("open", &ProcessClass::OpenProcess)
      .Build();
  }

  DISALLOW_COPY_AND_ASSIGN(ProcessClass);
};

//////////////////////////////////////////////////////////////////////
//
// OpenProcessCallback
//
class OpenProcessCallback : public v8_glue::PromiseCallback {
  public: OpenProcessCallback(v8_glue::Runner* runner)
    : v8_glue::PromiseCallback(runner) {
  }
  public: ~OpenProcessCallback() = default;

  public: void Run(domapi::IoContextId context_id, int error_code) {
    if (error_code) {
      Reject(FileError(error_code));
      return;
    }
    Resolve(new Process(context_id));
  }

  DISALLOW_COPY_AND_ASSIGN(OpenProcessCallback);
};

v8::Handle<v8::Object> ProcessClass::OpenProcess(
    const base::string16& command_line) {
  auto const runner = ScriptController::instance()->runner();
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);
  auto const callback = make_scoped_refptr(
      new OpenProcessCallback(runner));
  ScriptController::instance()->io_delegate()->NewProcess(
    command_line, base::Bind(&OpenProcessCallback::Run, callback));
  return runner_scope.Escape(callback->GetPromise(runner->isolate()));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Process
//
DEFINE_SCRIPTABLE_OBJECT(Process, ProcessClass);

Process::Process(domapi::IoContextId context_id)
    : ScriptableBase(context_id) {
}

Process::~Process() {
}

}  // namespace os
}  // namespace dom
