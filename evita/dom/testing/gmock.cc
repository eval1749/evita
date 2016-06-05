// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "evita/dom/testing/gmock.h"

#include "evita/dom/converter.h"
#include "evita/dom/testing/abstract_dom_test.h"
#include "evita/dom/testing/mock_io_delegate.h"
#include "evita/dom/testing/mock_view_impl.h"
#include "gin/object_template_builder.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace dom {

namespace {

using ::testing::_;

void ExpectCallCreateTextWindow(int times) {
  EXPECT_CALL(*AbstractDomTest::GetInstance()->mock_view_impl(),
              CreateTextWindow(_))
      .Times(times);
}

void SetSpellingSuggestions(int error_code,
                            const std::vector<base::string16> strings) {
  AbstractDomTest::GetInstance()->mock_io_delegate()->SetStrings(
      "GetSpellingSuggestions", error_code, strings);
}

}  // namespace

// Install testing.gmock
void GMock::Install(v8::Isolate* isolate,
                    v8::Local<v8::ObjectTemplate> testing) {
  const auto& gmock =
      gin::ObjectTemplateBuilder(isolate)
          .SetMethod("expectCallCreateTextWindow", ExpectCallCreateTextWindow)
          .SetMethod("setSpellingSuggestions", SetSpellingSuggestions)
          .Build();
  testing->Set(gin::StringToV8(isolate, "gmock"), gmock);
}

}  // namespace dom
