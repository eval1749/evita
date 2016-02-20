// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include "evita/dom/testing/test_runner.h"

#include "evita/dom/converter.h"
#include "evita/dom/lock.h"
#include "evita/dom/testing/abstract_dom_test.h"
#include "gin/object_template_builder.h"

namespace dom {

namespace {

void RunMicrotasks() {
  DOM_AUTO_UNLOCK_SCOPE();
  const auto test = AbstractDomTest::GetInstance();
  test->RunMessageLoopUntilIdle();
}

}  // namespace

void TestRunner::Install(v8::Isolate* isolate,
                         v8::Local<v8::ObjectTemplate> global) {
  auto test_runner = gin::ObjectTemplateBuilder(isolate)
                         .SetMethod("runMicrotasks", RunMicrotasks)
                         .Build();
  global->Set(gin::StringToV8(isolate, "testRunner"), test_runner);
}

}  // namespace dom
