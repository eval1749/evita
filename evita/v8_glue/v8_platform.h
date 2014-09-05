// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_v8_glue_v8_platform_h)
#define INCLUDE_evita_v8_glue_v8_platform_h

#include "common/memory/singleton.h"
#include "v8/include/v8-platform.h"

namespace v8_glue {

//////////////////////////////////////////////////////////////////////
//
// V8Platform
//
class V8Platform : public common::Singleton<V8Platform>, public v8::Platform {
  DECLARE_SINGLETON_CLASS(V8Platform);

  private: V8Platform();
  public: virtual ~V8Platform();

  // v8::Platform
  private: virtual void CallOnBackgroundThread(
      v8::Task* task, v8::Platform::ExpectedRuntime expected_runtime) override;
  private: virtual void CallOnForegroundThread(
      v8::Isolate* isolate, v8::Task* task) override;
};

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_v8_platform_h)
