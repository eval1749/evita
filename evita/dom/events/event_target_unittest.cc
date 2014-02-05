// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

#include "evita/dom/events/event.h"
#include "evita/dom/events/event_target.h"

namespace {

using v8_glue::Nullable;

//////////////////////////////////////////////////////////////////////
//
// SampleEventTarget
//
class SampleEventTarget
    : public v8_glue::Scriptable<SampleEventTarget, dom::EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(SampleEventTarget);
  friend class SampleEventTargetClass;

  private: gc::Member<dom::Event> handled_;

  public: SampleEventTarget() = default;
  public: virtual ~SampleEventTarget() = default;

  public: Nullable<dom::Event> handled() const { return handled_.get(); }
  public: void set_handled(Nullable<dom::Event> event) {
    handled_ = event;
  }

  public: void HandleEvent(dom::Event* event) {
    handled_= event;
  }

  DISALLOW_COPY_AND_ASSIGN(SampleEventTarget);
};

//////////////////////////////////////////////////////////////////////
//
// SampleEventTargetClass
//
class SampleEventTargetClass :
    public v8_glue::DerivedWrapperInfo<SampleEventTarget, dom::EventTarget> {

  public: explicit SampleEventTargetClass(const char* name)
      : BaseClass(name) {
  }
  public: ~SampleEventTargetClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &SampleEventTargetClass::NewSampleEventTarget);
  }

  private: static SampleEventTarget* NewSampleEventTarget() {
    return new SampleEventTarget();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    BaseClass::SetupInstanceTemplate(builder);
    builder
      .SetProperty("handled", &SampleEventTarget::handled,
                   &SampleEventTarget::set_handled)
      .SetMethod("handleEvent", &SampleEventTarget::HandleEvent);
  }
};

DEFINE_SCRIPTABLE_OBJECT(SampleEventTarget, SampleEventTargetClass);

//////////////////////////////////////////////////////////////////////
//
// EventTargetTest
//
class EventTargetTest : public dom::AbstractDomTest {
  protected: EventTargetTest() {
  }
  public: virtual ~EventTargetTest() {
  }

  private: void virtual PopulateGlobalTemplate(
      v8::Isolate* isolate,
      v8::Handle<v8::ObjectTemplate> global_template) override {
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
  EXPECT_SCRIPT_EQ("Error: InvalidStateError",
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

}  // namespace
