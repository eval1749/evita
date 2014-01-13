// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/dom/events/event_handler.h"
#include "evita/dom/global.h"
#include "evita/dom/lock.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_controller.h"

namespace dom {

using ::testing::_;

namespace {

base::string16 V8ToString(v8::Handle<v8::Value> value) {
  v8::String::Value string_value(value);
  if (!string_value.length())
    return base::string16();
  return base::string16(reinterpret_cast<base::char16*>(*string_value),
                        string_value.length());
}

}  // namespace

AbstractDomTest::AbstractDomTest()
      : mock_view_impl_(new MockViewImpl()),
        script_controller_(nullptr) {
}

AbstractDomTest::~AbstractDomTest() {
}

v8::Isolate* AbstractDomTest::isolate() const {
  return v8::Isolate::GetCurrent();
}

ViewEventHandler* AbstractDomTest::view_event_handler() const {
  return script_controller_->event_handler();
}

bool AbstractDomTest::DoCall(v8::Isolate* isolate,
                             const base::StringPiece& name,
                             const Argv& argv) {
  auto callee = isolate->GetCurrentContext()->Global()->Get(
      gin::StringToV8(isolate, name));
  v8::TryCatch try_catch;
  auto result = callee->ToObject()->CallAsFunction(v8::Null(isolate),
      argv.size(), const_cast<v8::Handle<v8::Value>*>(argv.data()));
  if (try_catch.HasCaught()) {
    LOG(0) << "Call " << name << " failed: " <<
        V8ToString(try_catch.Exception());
  }
  return !result.IsEmpty();
}

std::string AbstractDomTest::EvalScript(const std::string& text) {
  auto eval_result = script_controller_->Evaluate(base::ASCIIToUTF16(text));
  if (eval_result.exception.length())
    return base::UTF16ToUTF8(eval_result.exception);
  return base::UTF16ToUTF8(eval_result.value);
}

void AbstractDomTest::PopulateGlobalTemplate(v8::Isolate*,
                                             v8::Handle<v8::ObjectTemplate>) {
}

bool AbstractDomTest::RunScript(const std::string& text) {
  auto eval_result = script_controller_->Evaluate(base::ASCIIToUTF16(text));
  if (!eval_result.exception.length())
    return true;
  LOG(0) << "RunScript: " << eval_result.exception;
  return false;
}

void AbstractDomTest::SetUp() {
  static int number_of_called;
  ++number_of_called;

  EXPECT_CALL(*mock_view_impl_, RegisterViewEventHandler(_))
    .Times(number_of_called == 1 ? 1 : 0);

  script_controller_ = dom::ScriptController::StartForTesting(
    mock_view_impl_.get());

  DOM_AUTO_LOCK_SCOPE();

  auto const isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  auto global_template = Global::instance()->object_template(isolate);
  PopulateGlobalTemplate(isolate, global_template);

  auto context = v8::Context::New(isolate, nullptr, global_template);
  context_.Reset(isolate, context);
  context->Enter();
  script_controller_->LoadJsLibrary();
}

void AbstractDomTest::TearDown() {
  auto const isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  auto context = v8::Local<v8::Context>::New(isolate, context_);
  context->Exit();
}

}  // namespace dom

#include "base/strings/utf_string_conversions.h"
