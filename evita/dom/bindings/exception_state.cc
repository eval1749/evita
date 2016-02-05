// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "evita/dom/bindings/exception_state.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "evita/dom/bindings/platform_error.h"
#include "evita/ginx/runner.h"
#include "gin/converter.h"
#include "gin/per_context_data.h"

namespace dom {

namespace {

base::string16 V8ToString(v8::Local<v8::Value> value) {
  v8::String::Value string_value(value);
  if (!string_value.length())
    return base::string16();
  return base::string16(reinterpret_cast<base::char16*>(*string_value),
                        static_cast<size_t>(string_value.length()));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ExceptionState
//
ExceptionState::ExceptionState(Situation situation,
                               v8::Local<v8::Context> context,
                               base::StringPiece interface_name,
                               base::StringPiece property_name)
    : context_(context),
      interface_name_(interface_name),
      property_name_(property_name),
      situation_(situation) {}

ExceptionState::ExceptionState(Situation situation,
                               v8::Local<v8::Context> context,
                               base::StringPiece interface_name)
    : ExceptionState(situation, context, interface_name, base::StringPiece()) {
  DCHECK(situation_ == Situation::Construction ||
         situation_ == Situation::DispatchEvent)
      << situation_;
}

ExceptionState::~ExceptionState() {}

std::string ExceptionState::ComposeMessage(const std::string& detail) const {
  const auto& interface_name = interface_name_.as_string();
  const auto& property_name = property_name_.as_string();
  switch (situation_) {
    case Situation::Construction:
      return base::StringPrintf("Failed to construct '%s': %s",
                                interface_name.c_str(), detail.c_str());
    case Situation::DispatchEvent:
      return base::StringPrintf("Failed to dispatch event '%s': %s",
                                interface_name.c_str(), detail.c_str());
    case Situation::MethodCall:
      return base::StringPrintf("Failed to execute '%s' on '%s': %s",
                                property_name.c_str(), interface_name.c_str(),
                                detail.c_str());
    case Situation::PropertyGet:
      return base::StringPrintf(
          "Failed to read the '%s' property from '%s': %s",
          property_name.c_str(), interface_name.c_str(), detail.c_str());
    case Situation::PropertySet:
      return base::StringPrintf("Failed to set the '%s' property on '%s': %s",
                                property_name.c_str(), interface_name.c_str(),
                                detail.c_str());
  }
  NOTREACHED() << "situation_=" << situation_;
  return std::string();
}

std::string ExceptionState::ComposeMessage(base::StringPiece detail) const {
  return ComposeMessage(detail.as_string());
}

void ExceptionState::ThrowArityError(int min_arity,
                                     int max_arity,
                                     int actual_arity) {
  if (min_arity == max_arity) {
    ThrowError(base::StringPrintf("Expect %d arguments, but %d supplied",
                                  min_arity, actual_arity));
    return;
  }
  ThrowError(base::StringPrintf("Expect %d to %d arguments, but %d supplied",
                                min_arity, max_arity, actual_arity));
}

void ExceptionState::ThrowArgumentError(const char* expected_type,
                                        v8::Local<v8::Value> value,
                                        int index) {
  ThrowTypeError(base::StringPrintf("Expect arguments[%d] as '%s' but '%ls'",
                                    index, expected_type,
                                    V8ToString(value).c_str()));
}

void ExceptionState::ThrowError(base::StringPiece detail) {
  DCHECK(!is_thrown_);
  is_thrown_ = true;
  const auto runner = ginx::Runner::From(context_);
  const auto isolate = runner->isolate();
  ginx::Runner::Scope runner_scope(runner);
  const auto exception =
      v8::Exception::Error(gin::StringToV8(isolate, ComposeMessage(detail)));
  isolate->ThrowException(exception);
}

void ExceptionState::ThrowException(v8::Local<v8::Value> exception) {
  DCHECK(!is_thrown_);
  is_thrown_ = true;
  const auto runner = ginx::Runner::From(context_);
  runner->isolate()->ThrowException(exception);
}

void ExceptionState::ThrowPlatformError(base::StringPiece api_name,
                                        int error_code) {
  DCHECK(!is_thrown_);
  is_thrown_ = true;
  const auto runner = ginx::Runner::From(context_);
  const auto isolate = runner->isolate();
  ginx::Runner::Scope runner_scope(runner);
  PlatformError error(api_name, error_code);
  isolate->ThrowException(gin::ConvertToV8(isolate, error));
}

void ExceptionState::ThrowRangeError(base::StringPiece detail) {
  DCHECK(!is_thrown_);
  is_thrown_ = true;
  const auto runner = ginx::Runner::From(context_);
  const auto isolate = runner->isolate();
  ginx::Runner::Scope runner_scope(runner);
  const auto exception = v8::Exception::RangeError(
      gin::StringToV8(isolate, ComposeMessage(detail)));
  isolate->ThrowException(exception);
}

void ExceptionState::ThrowReceiverError(v8::Local<v8::Value> value) {
  ThrowTypeError(base::StringPrintf("Expect receiver as %s but %ls.",
                                    interface_name_.as_string().c_str(),
                                    V8ToString(value).c_str()));
}

void ExceptionState::ThrowTypeError(base::StringPiece detail) {
  DCHECK(!is_thrown_);
  is_thrown_ = true;
  const auto runner = ginx::Runner::From(context_);
  const auto isolate = runner->isolate();
  ginx::Runner::Scope runner_scope(runner);
  const auto exception = v8::Exception::TypeError(
      gin::StringToV8(isolate, ComposeMessage(detail)));
  isolate->ThrowException(exception);
}

std::ostream& operator<<(std::ostream& ostream,
                         ExceptionState::Situation situation) {
  const char* const texts[] = {
#define V(name) #name,
      FOR_EACH_EXCEPTION_STATE_SITUATION(V)
#undef V
  };
  const auto& it = std::begin(texts) + static_cast<size_t>(situation);
  if (it < std::begin(texts) || it >= std::end(texts))
    return ostream << "???";
  return ostream << *it;
}

}  // namespace dom
