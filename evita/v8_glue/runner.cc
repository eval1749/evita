// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/v8_glue/runner.h"

#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/runner_delegate.h"

namespace v8_glue {

//////////////////////////////////////////////////////////////////////
//
// Runner::Scope
//
Runner::Scope::Scope(Runner* runner)
    : isolate_scope_(runner->isolate()),
      handle_scope_(runner->isolate()),
      context_scope_(runner->context()) {
}

Runner::Scope::~Scope() {
}

//////////////////////////////////////////////////////////////////////
//
// Runner
//
Runner::Runner(v8::Isolate* isoalte, RunnerDelegate* delegate)
    : gin::ContextHolder(isoalte),
      delegate_(delegate),
      weak_factory_(this) {
  v8::Isolate::Scope isolate_scope(isolate());
  v8::HandleScope handle_scope(isolate());
  auto const context = v8::Context::New(isolate(), nullptr,
      delegate_->GetGlobalTemplate(this));
  SetContext(context);

  v8::Context::Scope scope(context);
  delegate_->DidCreateContext(this);
}

Runner::~Runner() {
}

v8::Handle<v8::Object> Runner::global() const {
  return context()->Global();
}

v8::Handle<v8::Value> Runner::Call(v8::Handle<v8::Object> callee,
      v8::Handle<v8::Object> receiver, const Args& args) {
  delegate_->WillRunScript(this);
  v8::TryCatch try_catch;
  try_catch.SetCaptureMessage(true);
  try_catch.SetVerbose(true);
  auto const value = callee->CallAsFunction(receiver,
      static_cast<int>(args.size()),
      const_cast<v8::Handle<v8::Value>*>(args.data()));
  delegate_->DidRunScript(this);
  if (try_catch.HasCaught())
    delegate_->UnhandledException(this, try_catch);
  return value;
}

base::WeakPtr<Runner> Runner::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

v8::Handle<v8::Value> Runner::Run(const base::string16& script_text,
    const base::string16& script_name) {
  v8::TryCatch try_catch;
  try_catch.SetCaptureMessage(true);
  try_catch.SetVerbose(true);
  auto const script = v8::Script::New(
      gin::StringToV8(isolate(), script_text)->ToString(),
      gin::StringToV8(isolate(), script_name)->ToString());
  if (try_catch.HasCaught())
    delegate_->UnhandledException(this, try_catch);
  return Run(script);
}

v8::Handle<v8::Value> Runner::Run(v8::Handle<v8::Script> script) {
  delegate_->WillRunScript(this);
  v8::TryCatch try_catch;
  try_catch.SetCaptureMessage(true);
  try_catch.SetVerbose(true);
  auto const value = script->Run();
  delegate_->DidRunScript(this);
  if (try_catch.HasCaught())
    delegate_->UnhandledException(this, try_catch);
  delegate_->DidRunScript(this);
  return value;
}

}  // namespace v8_glue
