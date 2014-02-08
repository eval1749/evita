// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/script_callback.h"

#include "evita/v8_glue/runner.h"

namespace v8_glue {
namespace internal {

ScriptClosure::ScriptClosure(base::WeakPtr<Runner> runner,
                             v8::Handle<v8::Function> function)
     : function_(runner->isolate(), function), runner_(runner) {
}

ScriptClosure::~ScriptClosure() {
}

v8::Isolate* ScriptClosure::isolate() const {
  return runner_->isolate();
}

Runner* ScriptClosure::runner() const {
  return runner_.get();
}

void ScriptClosure::Run(Argv argv) {
  auto const isolate = runner_->isolate();
  auto function = v8::Local<v8::Function>::New(isolate, function_);
  runner_->Call(function, v8::Undefined(isolate), argv);
}

}  // namespace internal
}  // namespace v8_glue
