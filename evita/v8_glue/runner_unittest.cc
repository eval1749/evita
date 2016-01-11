// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/v8_glue/runner.h"

#include <string>

#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/isolate_holder.h"
#include "evita/v8_glue/runner_delegate.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class RunnerTest : public ::testing::Test {
 private:
  v8_glue::IsolateHolder isolate_holder_;

 private:
  v8_glue::RunnerDelegate delegate;

 private:
  v8_glue::Runner runner_;

 public:
  RunnerTest() : runner_(isolate_holder_.isolate(), &delegate) {}

 public:
  ~RunnerTest() = default;

 public:
  v8::Local<v8::Object> global() const { return runner_.global(); }

 public:
  v8::Isolate* isolate() { return isolate_holder_.isolate(); }

 public:
  v8_glue::Runner* runner() { return &runner_; }

 public:
  v8::Local<v8::Value> Get(const std::string& key) {
    return runner_.global()->Get(gin::StringToV8(isolate(), key));
  }
};

base::string16 V8ToString(v8::Local<v8::Value> value) {
  v8::String::Value string_value(value);
  if (!string_value.length())
    return base::string16();
  return base::string16(reinterpret_cast<base::char16*>(*string_value),
                        static_cast<size_t>(string_value.length()));
}

TEST_F(RunnerTest, Call) {
  v8_glue::Runner::Scope runner_scope(runner());
  runner()->Run(L"function foo(x, y) { return x + y; }", L"bar.js");
  auto const value =
      runner()->Call(Get("foo")->ToObject(), runner()->global(),
                     v8_glue::Runner::Args{v8::Integer::New(isolate(), 1),
                                           v8::Integer::New(isolate(), 2)});
  EXPECT_EQ(L"3", V8ToString(value));
}

TEST_F(RunnerTest, Run) {
  v8_glue::Runner::Scope runner_scope(runner());
  runner()->Run(L"this.foo = 'PASS';", L"bar.js");
  EXPECT_EQ(L"PASS", V8ToString(Get("foo")));
}

}  // namespace
