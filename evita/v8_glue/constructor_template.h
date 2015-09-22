// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#ifndef EVITA_V8_GLUE_CONSTRUCTOR_TEMPLATE_H_
#define EVITA_V8_GLUE_CONSTRUCTOR_TEMPLATE_H_

#include "evita/v8_glue/v8_glue.h"

namespace v8_glue {
class AbstractScriptable;

namespace internal {

void FinishConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info,
                         AbstractScriptable* impl);
bool IsValidConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info);

}  // namespace internal
}  // namespace v8_glue

#endif  // EVITA_V8_GLUE_CONSTRUCTOR_TEMPLATE_H_
