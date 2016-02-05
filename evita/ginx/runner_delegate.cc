// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ginx/runner_delegate.h"

namespace ginx {

RunnerDelegate::RunnerDelegate() {}

RunnerDelegate::~RunnerDelegate() {}

void RunnerDelegate::DidCreateContext(Runner* runner) {}

void RunnerDelegate::DidRunScript(Runner* runner) {}

v8::Local<v8::ObjectTemplate> RunnerDelegate::GetGlobalTemplate(
    Runner* runner) {
  return v8::Local<v8::ObjectTemplate>();
}

void RunnerDelegate::UnhandledException(Runner* runner,
                                        const v8::TryCatch& try_catch) {}

void RunnerDelegate::WillRunScript(Runner* runner) {}

}  // namespace ginx
