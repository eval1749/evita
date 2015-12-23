// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "evita/dom/testing/gtest.h"
#include "gin/object_template_builder.h"
#include "gtest/gtest.h"

namespace dom {

base::string16 V8ToString(v8::Handle<v8::Value> value);

namespace {

void ExpectEqual(const v8::Local<v8::Value> expected,
                 const v8::Local<v8::Value> actual,
                 const std::string& description) {
  EXPECT_TRUE(expected->StrictEquals(actual)) << description;
}

void ExpectFalse(bool condition, const std::string& description) {
  EXPECT_FALSE(condition) << description;
}

void ExpectTrue(bool condition, const std::string& description) {
  EXPECT_TRUE(condition) << description;
}

void Fail(const std::string& description) {
  FAIL() << description;
}

void Log(gin::Arguments* args) {
  base::string16 message;
  for (;;) {
    v8::Local<v8::Value> value;
    if (!args->GetNext(&value))
      break;
    if (!message.empty())
      message += L" ";
    message += V8ToString(value);
  }
  LOG(0) << message;
}

void Succeed() {
  SUCCEED();
}

}  // namespace

// Install testing.gtest
void GTest::Install(v8::Isolate* isolate,
                    v8::Local<v8::ObjectTemplate> global) {
  auto const testing = v8::ObjectTemplate::New(isolate);
  global->Set(gin::StringToV8(isolate, "testing"), testing);

  auto gtest = v8::ObjectTemplate::New(isolate);
  gtest = gin::ObjectTemplateBuilder(isolate)
              .SetMethod("expectEqual", ExpectEqual)
              .SetMethod("expectFalse", ExpectTrue)
              .SetMethod("expectTrue", ExpectTrue)
              .SetMethod("fail", Fail)
              .SetMethod("log", Log)
              .SetMethod("succeed", Succeed)
              .Build();
  testing->Set(gin::StringToV8(isolate, "gtest"), gtest);
}

}  // namespace dom
