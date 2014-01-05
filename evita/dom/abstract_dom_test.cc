// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

#include "base/strings/utf_string_conversions.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_controller.h"

namespace dom {

using ::testing::_;

AbstractDomTest::AbstractDomTest()
      : mock_view_impl_(new MockViewImpl()),
        script_controller_(nullptr) {
}

AbstractDomTest::~AbstractDomTest() {
}

ViewEventHandler* AbstractDomTest::view_event_handler() const {
  return script_controller_;
}

std::string AbstractDomTest::RunScript(const std::string& text) {
  auto eval_result = script_controller_->Evaluate(base::ASCIIToUTF16(text));
  if (eval_result.exception.length())
    return base::UTF16ToUTF8(eval_result.exception);
  return base::UTF16ToUTF8(eval_result.value);
}

void AbstractDomTest::SetUp() {
  static int number_of_called;
  ++number_of_called;

  EXPECT_CALL(*mock_view_impl_, RegisterViewEventHandler(_))
    .Times(number_of_called == 1 ? 1 : 0);

  script_controller_ = dom::ScriptController::StartForTesting(
    mock_view_impl_.get());

  auto const isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  auto global_template = v8::ObjectTemplate::New(isolate);
  {
    v8::HandleScope handle_scope(isolate);
    v8::Context::Scope context_scope(v8::Context::New(isolate));
    ScriptController::instance()->PopulateGlobalTemplate(
        isolate, global_template);
    PopulateGlobalTemplate(isolate, global_template);
  }

  auto context = v8::Context::New(isolate, nullptr, global_template);
  context_.Reset(isolate, context);
  context->Enter();
  script_controller_->LoadJsLibrary();
}

void AbstractDomTest::PopulateGlobalTemplate(v8::Isolate*,
                                             v8::Handle<v8::ObjectTemplate>) {
}

void AbstractDomTest::TearDown() {
  auto const isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  auto context = v8::Local<v8::Context>::New(isolate, context_);
  context->Exit();
}

}  // namespace dom

#include "base/strings/utf_string_conversions.h"
