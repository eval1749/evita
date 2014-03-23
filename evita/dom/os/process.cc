// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE process.

#include "evita/dom/os/process.h"

#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "evita/dom/converter.h"
#include "evita/dom/promise_deferred.h"
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
struct Converter<domapi::ProcessId> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      domapi::ProcessId context_id) {
    return gin::ConvertToV8(isolate, new dom::os::Process(context_id));
  }
};
}  // namespace gin


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
    ScriptHost::instance()->ThrowError(
        "Cannot create an instance of Process.");
    return nullptr;
  }

  private: static v8::Handle<v8::Promise> OpenProcess(
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

v8::Handle<v8::Promise> ProcessClass::OpenProcess(
    const base::string16& command_line) {
  return PromiseDeferred::Call(base::Bind(
      &domapi::IoDelegate::OpenProcess,
      base::Unretained(ScriptHost::instance()->io_delegate()),
      command_line));
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
