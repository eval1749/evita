// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <string>
#include <vector>

#include "evita/dom/testing/test_runner.h"

#include "evita/dom/converter.h"
#include "evita/dom/lock.h"
#include "evita/dom/testing/abstract_dom_test.h"
#include "evita/dom/testing/mock_io_delegate.h"
#include "gin/object_template_builder.h"

namespace dom {

namespace {

void RunMicrotasks() {
  DOM_AUTO_UNLOCK_SCOPE();
  const auto test = AbstractDomTest::GetInstance();
  test->RunMessageLoopUntilIdle();
}

void SetOpenResult(const std::string& name, int error_code) {
  const auto test = AbstractDomTest::GetInstance();
  test->mock_io_delegate()->SetOpenResult(name, error_code);
}

void SetResource(const base::string16& type,
                 const base::string16& name,
                 const std::vector<uint8_t>& data) {
  const auto test = AbstractDomTest::GetInstance();
  test->mock_io_delegate()->SetResource(type, name, data);
}

void SetStringsResult(const std::string& operation,
                      int error_code,
                      const std::vector<base::string16>& data) {
  const auto test = AbstractDomTest::GetInstance();
  test->mock_io_delegate()->SetStrings(operation, error_code, data);
}

}  // namespace

void TestRunner::Install(v8::Isolate* isolate,
                         v8::Local<v8::ObjectTemplate> global) {
  auto test_runner = gin::ObjectTemplateBuilder(isolate)
                         .SetMethod("runMicrotasks", RunMicrotasks)
                         .SetMethod("setOpenResult", SetOpenResult)
                         .SetMethod("setResource", SetResource)
                         .SetMethod("setStringsResult", SetStringsResult)
                         .Build();
  global->Set(gin::StringToV8(isolate, "testRunner"), test_runner);
}

}  // namespace dom
