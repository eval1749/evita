// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

#include "evita/dom/events/event.h"
#include "evita/dom/events/event_target.h"
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
  v8::Handle<v8::FunctionTemplate> CreateConstructorTemplate(
      v8::Isolate* isolate) final {
    return v8_glue::CreateConstructorTemplate(
        isolate, &SampleEventTargetClass::NewSampleEventTarget);
  }

  static SampleEventTarget* NewSampleEventTarget() {
    return new SampleEventTarget();
  }

  v8::Handle<v8::ObjectTemplate> SetupInstanceTemplate(
      v8::Isolate* isolate,
      v8::Handle<v8::ObjectTemplate> templ) final {
    auto const base_templ = BaseClass::SetupInstanceTemplate(isolate, templ);
    gin::ObjectTemplateBuilder builder(isolate, base_templ);
    builder.SetProperty("handled", &SampleEventTarget::handled,
                        &SampleEventTarget::set_handled)
        .SetMethod("handleEvent", &SampleEventTarget::HandleEvent);
    return builder.Build();
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
      v8::Handle<v8::ObjectTemplate> global_template) final {
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
  EXPECT_SCRIPT_EQ("Error: InvalidStateError", "sample.dispatchEvent(event)");

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
