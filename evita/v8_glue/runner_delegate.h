// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_V8_GLUE_RUNNER_DELEGATE_H_
#define EVITA_V8_GLUE_RUNNER_DELEGATE_H_

#include "base/macros.h"
#include "evita/v8_glue/v8.h"

namespace v8_glue {

class Runner;

class RunnerDelegate {
 public:
  virtual ~RunnerDelegate();

  virtual void DidCreateContext(Runner* runner);
  virtual void DidRunScript(Runner* runner);
  virtual v8::Local<v8::ObjectTemplate> GetGlobalTemplate(Runner* runner);
  virtual void UnhandledException(Runner* runner,
                                  const v8::TryCatch& try_catch);
  virtual void WillRunScript(Runner* runner);

 protected:
  RunnerDelegate();

 private:
  DISALLOW_COPY_AND_ASSIGN(RunnerDelegate);
};

}  // namespace v8_glue

#endif  // EVITA_V8_GLUE_RUNNER_DELEGATE_H_
