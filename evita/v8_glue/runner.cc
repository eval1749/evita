// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/v8_glue/runner.h"

#include "base/auto_reset.h"
#include "base/trace_event/trace_event.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/runner_delegate.h"
#include "evita/v8_glue/per_isolate_data.h"

namespace v8_glue {

namespace {
const int kMaxCallDepth = 20;
}

//////////////////////////////////////////////////////////////////////
//
// Runner::CurrentRunnerScope
//
Runner::CurrentRunnerScope::CurrentRunnerScope(Runner* runner)
    : isolate_(runner->isolate()) {
  PerIsolateData::From(isolate_)->set_current_runner(runner);
}

Runner::CurrentRunnerScope::~CurrentRunnerScope() {
  PerIsolateData::From(isolate_)->set_current_runner(nullptr);
}

//////////////////////////////////////////////////////////////////////
//
// Runner::EscapableHandleScope
//
Runner::EscapableHandleScope::EscapableHandleScope(Runner* runner)
    : isolate_locker_(runner->isolate()),
      isolate_scope_(runner->isolate()),
      handle_scope_(runner->isolate()),
      context_scope_(runner->context()),
      current_runner_scope_(runner),
      runner_(runner) {
#if defined(_DEBUG)
  ++runner->in_scope_;
#endif
}

Runner::EscapableHandleScope::~EscapableHandleScope() {
#if defined(_DEBUG)
  --runner_->in_scope_;
#endif
}

//////////////////////////////////////////////////////////////////////
//
// Runner::Scope
//
Runner::Scope::Scope(Runner* runner)
    : isolate_locker_(runner->isolate()),
      isolate_scope_(runner->isolate()),
      handle_scope_(runner->isolate()),
      context_scope_(runner->context()),
      current_runner_scope_(runner),
      runner_(runner) {
#if defined(_DEBUG)
  ++runner->in_scope_;
#endif
}

Runner::Scope::~Scope() {
#if defined(_DEBUG)
  --runner_->in_scope_;
#endif
}

//////////////////////////////////////////////////////////////////////
//
// Runner
//
Runner::Runner(v8::Isolate* isolate, RunnerDelegate* delegate)
    : gin::ContextHolder(isolate),
      call_depth_(0),
      delegate_(delegate),
#if defined(_DEBUG)
      in_scope_(false),
#endif
      weak_factory_(this) {
  v8::Locker locker_scope(isolate);
  v8::Isolate::Scope isolate_scope(isolate);
  v8::HandleScope handle_scope(isolate);
  auto const context =
      v8::Context::New(isolate, nullptr, delegate_->GetGlobalTemplate(this));
  SetContext(context);

  v8::Context::Scope scope(context);
  delegate_->DidCreateContext(this);
}

Runner::~Runner() {}

Runner* Runner::current_runner(v8::Isolate* isolate) {
  return PerIsolateData::From(isolate)->current_runner();
}

v8::Handle<v8::Object> Runner::global() const {
  return context()->Global();
}

v8::Handle<v8::Value> Runner::Call(v8::Handle<v8::Value> callee,
                                   v8::Handle<v8::Value> receiver,
                                   const Args& args) {
#if defined(_DEBUG)
  DCHECK(in_scope_);
#endif
  base::AutoReset<int> call_depth(&call_depth_, call_depth_ + 1);
  if (!CheckCallDepth())
    return v8::Handle<v8::Value>();
  delegate_->WillRunScript(this);
  v8::MaybeLocal<v8::Value> maybe_value;
  v8::TryCatch try_catch;
  {
    TRACE_EVENT0("script", "Runner::Call");
    maybe_value = callee->ToObject()->CallAsFunction(
        context(), receiver, static_cast<int>(args.size()),
        const_cast<v8::Handle<v8::Value>*>(args.data()));
  }
  delegate_->DidRunScript(this);
  HandleTryCatch(try_catch);
  if (maybe_value.IsEmpty())
    return v8::Handle<v8::Value>();
  return maybe_value.ToLocalChecked();
}

v8::Handle<v8::Value> Runner::CallAsConstructor(v8::Handle<v8::Value> callee,
                                                const Args& args) {
#if defined(_DEBUG)
  DCHECK(in_scope_);
#endif
  delegate_->WillRunScript(this);
  v8::MaybeLocal<v8::Value> maybe_value;
  v8::TryCatch try_catch;
  {
    TRACE_EVENT0("script", "Runner::CallAsConstructor");
    maybe_value = callee->ToObject()->CallAsConstructor(
        context(), static_cast<int>(args.size()),
        const_cast<v8::Handle<v8::Value>*>(args.data()));
  }
  delegate_->DidRunScript(this);
  HandleTryCatch(try_catch);
  if (maybe_value.IsEmpty())
    return v8::Handle<v8::Value>();
  return maybe_value.ToLocalChecked();
}

bool Runner::CheckCallDepth() {
  if (call_depth_ < kMaxCallDepth)
    return true;
  auto const isolate = this->isolate();
  isolate->ThrowException(v8::Exception::RangeError(
      gin::StringToV8(isolate, "Maximum call stack size exceeded")));
  return false;
}

v8::Handle<v8::Script> Runner::Compile(const base::string16& script_text,
                                       const base::string16& script_name) {
  v8::TryCatch try_catch;
  v8::ScriptOrigin script_origin(gin::StringToV8(isolate(), script_name));
  v8::ScriptCompiler::Source source(gin::StringToV8(isolate(), script_text),
                                    script_origin);
  auto script = v8::ScriptCompiler::Compile(context(), &source);
  if (script.IsEmpty()) {
    HandleTryCatch(try_catch);
    return v8::Handle<v8::Script>();
  }
  return script.ToLocalChecked();
}

v8::Handle<v8::Value> Runner::GetGlobalProperty(const base::StringPiece& name) {
#if defined(_DEBUG)
  DCHECK(in_scope_);
#endif
  return global()->Get(gin::StringToV8(isolate(), name));
}

base::WeakPtr<Runner> Runner::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

void Runner::HandleTryCatch(const v8::TryCatch& try_catch) {
  if (!try_catch.HasCaught())
    return;
  delegate_->UnhandledException(this, try_catch);
}

v8::Handle<v8::Value> Runner::Run(const base::string16& script_text,
                                  const base::string16& script_name) {
#if defined(_DEBUG)
  DCHECK(in_scope_);
#endif
  auto const script = Compile(script_text, script_name);
  if (script.IsEmpty())
    return v8::Handle<v8::Value>();
  return Run(script);
}

v8::Handle<v8::Value> Runner::Run(v8::Handle<v8::Script> script) {
#if defined(_DEBUG)
  DCHECK(in_scope_);
#endif
  base::AutoReset<int> call_depth(&call_depth_, call_depth_ + 1);
  if (!CheckCallDepth())
    return v8::Handle<v8::Value>();
  delegate_->WillRunScript(this);
  v8::TryCatch try_catch;
  auto const value = script->Run();
  delegate_->DidRunScript(this);
  HandleTryCatch(try_catch);
  delegate_->DidRunScript(this);
  return value;
}

}  // namespace v8_glue
