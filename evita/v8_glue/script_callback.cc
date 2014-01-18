// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/script_callback.h"

namespace v8_glue {
namespace internal {

ScriptClosure::ScriptClosure(v8::Isolate* isolate,
                             v8::Handle<v8::Function> function)
     : function_(isolate, function), isolate_(isolate) {
}

ScriptClosure::~ScriptClosure() {
}

void ScriptClosure::Run(Argv argv) {
  auto function = v8::Local<v8::Function>::New(isolate_, function_);
  function->CallAsFunction(v8::Null(isolate_), static_cast<int>(argv.size()),
                           argv.data());
}

}  // namespace internal
}  // namespace v8_glue
