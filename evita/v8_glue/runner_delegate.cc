// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/v8_glue/runner_delegate.h"

namespace v8_glue {

RunnerDelegate::RunnerDelegate() {
}

RunnerDelegate::~RunnerDelegate() {
}

void RunnerDelegate::DidCreateContext(Runner*) {
}

void RunnerDelegate::DidRunScript(Runner*) {
}

v8::Handle<v8::ObjectTemplate> RunnerDelegate::GetGlobalTemplate(Runner*) {
  return v8::Handle<v8::ObjectTemplate>();
}

void RunnerDelegate::UnhandledException(Runner*, const v8::TryCatch&) {
}

void RunnerDelegate::WillRunScript(Runner*) {
}

}  // namespace v8_glue
