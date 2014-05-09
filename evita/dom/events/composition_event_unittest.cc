// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

#include <vector>

#include "evita/bindings/CompositionEventInit.h"
#include "evita/dom/events/composition_event.h"
#include "evita/dom/public/text_composition_event.h"

namespace {

class CompositionEventTest : public dom::AbstractDomTest {
  protected: CompositionEventTest() {
  }
  public: virtual ~CompositionEventTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(CompositionEventTest);
};

TEST_F(CompositionEventTest, ctor_init_dict) {
  RunnerScope runner_scope(this);
  EXPECT_SCRIPT_VALID("var event;"
                      "function init(x) { event = x; }");
  dom::CompositionEventInit init_dict;
  init_dict.set_bubbles(true);
  init_dict.set_cancelable(true);
  init_dict.set_attributes(std::vector<uint8_t> {1, 2, 3});
  init_dict.set_caret(42);
  init_dict.set_data(L"foo");
  auto const event = new dom::CompositionEvent(L"compositionupdate", init_dict);
  EXPECT_SCRIPT_VALID_CALL("init", event->GetWrapper(isolate()));
  EXPECT_SCRIPT_TRUE("event.bubbles");
  EXPECT_SCRIPT_TRUE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.currentTarget == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("compositionupdate", "event.type");
  EXPECT_SCRIPT_EQ("0", "event.detail");
  EXPECT_SCRIPT_TRUE("event.view == null");
  EXPECT_SCRIPT_VALID("var attrs = event.attributes;");
  EXPECT_SCRIPT_EQ("3", "attrs.length");
  EXPECT_SCRIPT_EQ("1 2 3", "attrs[0] + ' ' + attrs[1] + ' ' + attrs[2]");
  EXPECT_SCRIPT_EQ("42", "event.caret");
  EXPECT_SCRIPT_EQ("foo", "event.data");
}

TEST_F(CompositionEventTest, ctor_event) {
  RunnerScope runner_scope(this);
  domapi::TextCompositionEvent api_event;
  api_event.event_type = domapi::EventType::TextCompositionStart;
  api_event.data.attributes = std::vector<uint8_t> {1, 2, 3};
  api_event.data.caret = 42;
  api_event.data.text = L"foo";
  EXPECT_SCRIPT_VALID("var event;"
                      "function init(x) { event = x; }");
  auto const event = new dom::CompositionEvent(api_event);
  EXPECT_SCRIPT_VALID_CALL("init", event->GetWrapper(isolate()));
  EXPECT_SCRIPT_TRUE("event.bubbles");
  EXPECT_SCRIPT_TRUE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.currentTarget == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("event.eventPhase == Event.PhaseType.NONE");
  EXPECT_SCRIPT_TRUE("event.timeStamp == 0");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("compositionstart", "event.type");
  EXPECT_SCRIPT_EQ("0", "event.detail");
  EXPECT_SCRIPT_TRUE("event.view == null");
  EXPECT_SCRIPT_VALID("var attrs = event.attributes;");
  EXPECT_SCRIPT_EQ("3", "attrs.length");
  EXPECT_SCRIPT_EQ("1 2 3", "attrs[0] + ' ' + attrs[1] + ' ' + attrs[2]");
  EXPECT_SCRIPT_EQ("42", "event.caret");
  EXPECT_SCRIPT_EQ("foo", "event.data");
}

}  // namespace
