// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>

#include "evita/ginx/runner.h"

#include "base/macros.h"
#include "base/message_loop/message_loop.h"
#include "evita/ginx/converter.h"
#include "evita/ginx/isolate_holder.h"
#include "evita/ginx/runner_delegate.h"
#include "gin/array_buffer.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace ginx {

namespace {

class EmptyRunnerDelegate : public RunnerDelegate {
 public:
  EmptyRunnerDelegate() = default;
  ~EmptyRunnerDelegate() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(EmptyRunnerDelegate);
};

}  // namespace

class RunnerTest : public ::testing::Test {
 public:
  RunnerTest();
  ~RunnerTest();

  Runner* runner() { return runner_.get(); }

 private:
  EmptyRunnerDelegate delegate_;
  std::unique_ptr<IsolateHolder> isolate_holder_;
  base::MessageLoop message_loop_;
  std::unique_ptr<Runner> runner_;

  DISALLOW_COPY_AND_ASSIGN(RunnerTest);
};

RunnerTest::RunnerTest() {
  gin::IsolateHolder::Initialize(gin::IsolateHolder::kStrictMode,
                                 gin::IsolateHolder::kStableV8Extras,
                                 gin::ArrayBufferAllocator::SharedInstance());
  v8::V8::InitializeICU();
  isolate_holder_.reset(new IsolateHolder());
  runner_.reset(new Runner(isolate_holder_->isolate(), &delegate_));
}

RunnerTest::~RunnerTest() {
  {
    v8::Isolate::Scope isolate_scope(runner_->isolate());
    runner_.reset();
  }
  isolate_holder_.reset();
}

base::string16 V8ToString(v8::Local<v8::Value> value) {
  v8::String::Value string_value(value);
  if (!string_value.length())
    return base::string16();
  return base::string16(reinterpret_cast<base::char16*>(*string_value),
                        static_cast<size_t>(string_value.length()));
}

TEST_F(RunnerTest, Call) {
  Runner::Scope runner_scope(runner());
  runner()->Run(L"function foo(x, y) { return x + y; }", L"bar.js");
  auto* const isolate = runner()->isolate();
  auto const value = runner()->CallAsFunction(
      runner()->GetGlobalProperty("foo")->ToObject(), runner()->global(),
      v8::Integer::New(isolate, 1), v8::Integer::New(isolate, 2));
  EXPECT_EQ(L"3", V8ToString(value));
}

TEST_F(RunnerTest, Run) {
  Runner::Scope runner_scope(runner());
  runner()->Run(L"this.foo = 'PASS';", L"bar.js");
  EXPECT_EQ(L"PASS", V8ToString(runner()->GetGlobalProperty("foo")));
}

}  // namespace ginx
