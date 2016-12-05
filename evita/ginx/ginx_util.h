// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GINX_GINX_UTIL_H_
#define EVITA_GINX_GINX_UTIL_H_

#include "v8/include/v8.h"

namespace ginx {

// Returns true if |object| is instance of |clazz|. This is C++ implementation
// of JavaScript operator |instanceof|.
bool IsInstanceOf(v8::Local<v8::Context> context,
                  v8::Local<v8::Object> object,
                  v8::Local<v8::Object> clazz);

// Populate Mozilla compiatible |fileName|, |lineNumber| and |columnNumber|
// properties to |Error| object if possible.
void ImproveErrorMessage(v8::Local<v8::Context> context,
                         const v8::TryCatch& try_catch);

}  // namespace ginx

#endif  // EVITA_GINX_GINX_UTIL_H_
