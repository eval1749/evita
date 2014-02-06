// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_v8_glue_script_runner_delegate_h)
#define INCLUDE_evita_v8_glue_script_runner_delegate_h

#include "base/basictypes.h"
#include "evita/v8_glue/v8.h"

namespace v8_glue {

class Runner;

class RunnerDelegate {
  public: RunnerDelegate();
  public: virtual ~RunnerDelegate();

  public: virtual void DidCreateContext(Runner* runner);
  public: virtual void DidRunScript(Runner* runner);
  public: virtual v8::Handle<v8::ObjectTemplate>
      GetGlobalTemplate(Runner* runner);
  public: virtual void UnhandledException(Runner* runner,
                                          const v8::TryCatch& try_catch);
  public: virtual void WillRunScript(Runner* runner);

  DISALLOW_COPY_AND_ASSIGN(RunnerDelegate);
};

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_script_runner_delegate_h)
