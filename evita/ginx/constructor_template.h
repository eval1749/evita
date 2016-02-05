// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#ifndef EVITA_GINX_CONSTRUCTOR_TEMPLATE_H_
#define EVITA_GINX_CONSTRUCTOR_TEMPLATE_H_

#include "evita/ginx/ginx.h"

namespace ginx {
class AbstractScriptable;

namespace internal {

void FinishConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info,
                         AbstractScriptable* impl);
bool IsValidConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info);

}  // namespace internal
}  // namespace ginx

#endif  // EVITA_GINX_CONSTRUCTOR_TEMPLATE_H_
