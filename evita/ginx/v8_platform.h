// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GINX_V8_PLATFORM_H_
#define EVITA_GINX_V8_PLATFORM_H_

#include "base/macros.h"
#include "common/memory/singleton.h"
#include "v8/include/v8-platform.h"

namespace ginx {

//////////////////////////////////////////////////////////////////////
//
// V8Platform
//
class V8Platform final : public common::Singleton<V8Platform>,
                         public v8::Platform {
  DECLARE_SINGLETON_CLASS(V8Platform);

 public:
  ~V8Platform() final;

 private:
  V8Platform();

  // v8::Platform
  void CallOnBackgroundThread(
      v8::Task* task,
      v8::Platform::ExpectedRuntime expected_runtime) final;
  void CallOnForegroundThread(v8::Isolate* isolate, v8::Task* task) final;

  DISALLOW_COPY_AND_ASSIGN(V8Platform);
};

}  // namespace ginx

#endif  // EVITA_GINX_V8_PLATFORM_H_
