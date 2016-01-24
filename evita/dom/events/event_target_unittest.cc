// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/events/event.h"
#include "evita/dom/events/event_target.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/nullable.h"

namespace dom {

using v8_glue::Nullable;

//////////////////////////////////////////////////////////////////////
//
// SampleEventTarget
//
class SampleEventTarget final
    : public v8_glue::Scriptable<SampleEventTarget, EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(SampleEventTarget);

 public:
  SampleEventTarget() = default;
  ~SampleEventTarget() final = default;

 public:
  Nullable<Event> handled() const { return handled_.get(); }
  void set_handled(Nullable<Event> event) { handled_ = event; }

  void HandleEvent(Event* event) { handled_ = event; }

 private:
  friend class SampleEventTargetClass;

  gc::Member<Event> handled_;

  DISALLOW_COPY_AND_ASSIGN(SampleEventTarget);
};

//////////////////////////////////////////////////////////////////////
//
// SampleEventTargetClass
//
class SampleEventTargetClass final
    : public v8_glue::DerivedWrapperInfo<SampleEventTarget, EventTarget> {
 public:
  explicit SampleEventTargetClass(const char* name) : BaseClass(name) {}
  ~SampleEventTargetClass() final = default;

 private:
  static void ConstructSampleEventTarget(
      const v8::FunctionCallbackInfo<v8::Value>& info) {
    if (!v8_glue::internal::IsValidConstructCall(info))
      return;
    v8_glue::internal::FinishConstructCall(info, NewSampleEventTarget(info));
  }

  v8::Local<v8::FunctionTemplate> CreateConstructorTemplate(
      v8::Isolate* isolate) final {
    return v8::FunctionTemplate::New(
        isolate, &SampleEventTargetClass::ConstructSampleEventTarget);
  }

  static SampleEventTarget* NewSampleEventTarget(
      const v8::FunctionCallbackInfo<v8::Value>& info) {
    return new SampleEventTarget();
  }

  v8::Local<v8::ObjectTemplate> SetupInstanceTemplate(
      v8::Isolate* isolate,
      v8::Local<v8::ObjectTemplate> base_templ) final {
    const auto templ = BaseClass::SetupInstanceTemplate(isolate, base_templ);
    templ->SetAccessorProperty(
        gin::StringToSymbol(isolate, "handled"),
        v8::FunctionTemplate::New(isolate, &GetHandled),
        v8::FunctionTemplate::New(isolate, &SetHandled),
        static_cast<v8::PropertyAttribute>(v8::DontDelete | v8::DontEnum));
    templ->Set(gin::StringToSymbol(isolate, "handleEvent"),
               v8::FunctionTemplate::New(isolate, &HandleEvent),
               static_cast<v8::PropertyAttribute>(v8::DontDelete |
                                                  v8::DontEnum | v8::ReadOnly));
    return templ;
  }

  // |handled| IDL attribute getter
  static void GetHandled(const v8::FunctionCallbackInfo<v8::Value>& info) {
    const auto isolate = info.GetIsolate();
    const auto context = isolate->GetCurrentContext();
    ExceptionState exception_state(ExceptionState::Situation::PropertyGet,
                                   context, "SampleEventTarget", "handled");
    SampleEventTarget* impl = nullptr;
    if (!gin::ConvertFromV8(isolate, info.This(), &impl)) {
      exception_state.ThrowReceiverError(info.This());
      return;
    }
    auto value = impl->handled();
    v8::Local<v8::Value> v8_value;
    if (!gin::TryConvertToV8(isolate, value, &v8_value))
      return;
    info.GetReturnValue().Set(v8_value);
  }

  // |handleEvent| IDL operation
  static void HandleEvent(const v8::FunctionCallbackInfo<v8::Value>& info) {
    const auto isolate = info.GetIsolate();
    const auto context = isolate->GetCurrentContext();
    ExceptionState exception_state(ExceptionState::Situation::PropertyGet,
                                   context, "SampleEventTarget", "handleEvent");
    if (info.Length() != 1) {
      exception_state.ThrowArityError(1, 1, info.Length());
      return;
    }
    SampleEventTarget* impl = nullptr;
    if (!gin::ConvertFromV8(isolate, info.This(), &impl)) {
      exception_state.ThrowReceiverError(info.This());
      return;
    }
    Event* value;
    if (!gin::ConvertFromV8(isolate, info[0], &value)) {
      exception_state.ThrowArgumentError("Event", info[0], 0);
      return;
    }
    impl->HandleEvent(value);
  }

  // |handled| IDL attribute setter
  static void SetHandled(const v8::FunctionCallbackInfo<v8::Value>& info) {
    const auto isolate = info.GetIsolate();
    const auto context = isolate->GetCurrentContext();
    ExceptionState exception_state(ExceptionState::Situation::PropertySet,
                                   context, "SampleEventTarget", "handled");
    if (info.Length() != 1) {
      exception_state.ThrowArityError(1, 1, info.Length());
      return;
    }
    SampleEventTarget* impl = nullptr;
    if (!gin::ConvertFromV8(isolate, info.This(), &impl)) {
      exception_state.ThrowReceiverError(info.This());
      return;
    }
    v8_glue::Nullable<Event> new_value;
    if (!gin::ConvertFromV8(isolate, info[0], &new_value)) {
      exception_state.ThrowArgumentError("Event or null", info[0], 0);
      return;
    }
    impl->set_handled(new_value);
  }

  DISALLOW_COPY_AND_ASSIGN(SampleEventTargetClass);
};

DEFINE_SCRIPTABLE_OBJECT(SampleEventTarget, SampleEventTargetClass);

//////////////////////////////////////////////////////////////////////
//
// EventTargetTest
//
class EventTargetTest : public AbstractDomTest {
 public:
  ~EventTargetTest() override = default;

 protected:
  EventTargetTest() = default;

 private:
  void PopulateGlobalTemplate(
      v8::Isolate* isolate,
      v8::Local<v8::ObjectTemplate> global_template) final {
    v8_glue::Installer<SampleEventTarget>::Run(isolate, global_template);
  }

  DISALLOW_COPY_AND_ASSIGN(EventTargetTest);
};

TEST_F(EventTargetTest, dispatchEvent_function) {
  EXPECT_SCRIPT_VALID(
      "var sample = new SampleEventTarget();"
      "var event = new Event('foo');"
      "var handled = false;"
      "function handler(event) { handled = event; }"
      "sample.addEventListener('foo', handler);"
      "sample.dispatchEvent(event);");
  EXPECT_SCRIPT_TRUE("handled == event");
  EXPECT_SCRIPT_TRUE("event.target == sample");

  // We can't dispatch again
  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'dispatchEvent' on 'EventTarget': "
      "InvalidStateError",
      "sample.dispatchEvent(event)");

  EXPECT_SCRIPT_VALID(
      "var event2 = new Event('foo');"
      "sample.removeEventListener('foo', handler);"
      "handled = false;"
      "sample.dispatchEvent(event2);");
  EXPECT_SCRIPT_TRUE("handled == false");
}

TEST_F(EventTargetTest, dispatchEvent_handleEvent) {
  EXPECT_SCRIPT_VALID(
      "var sample = new SampleEventTarget();"
      "var event = new Event('foo');"
      "sample.addEventListener('foo', sample);"
      "sample.dispatchEvent(event);");
  EXPECT_SCRIPT_TRUE("sample.handled == event");
  EXPECT_SCRIPT_TRUE("event.target == sample");

  EXPECT_SCRIPT_VALID(
      "var event2 = new Event('foo');"
      "sample.removeEventListener('foo', sample);"
      "sample.handled = null;"
      "sample.dispatchEvent(event2);");
  EXPECT_SCRIPT_TRUE("sample.handled == null");
}

}  // namespace dom
