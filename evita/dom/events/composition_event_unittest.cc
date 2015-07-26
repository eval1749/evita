// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

#include <vector>

#include "evita/bindings/v8_glue_CompositionEventInit.h"
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
  std::vector<dom::CompositionSpan*> spans {
    new dom::CompositionSpan(0, 1, 1),
    new dom::CompositionSpan(1, 2, 2),
    new dom::CompositionSpan(2, 3, 3),
  };
  init_dict.set_spans(spans);
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
  EXPECT_SCRIPT_VALID("var spans = event.spans;");
  EXPECT_SCRIPT_EQ("3", "spans.length");
  EXPECT_SCRIPT_EQ("1 2 3",
    "spans[0].data + ' ' + spans[1].data + ' ' + spans[2].data");
  EXPECT_SCRIPT_EQ("42", "event.caret");
  EXPECT_SCRIPT_EQ("foo", "event.data");
}

TEST_F(CompositionEventTest, ctor_event) {
  RunnerScope runner_scope(this);
  domapi::TextCompositionEvent api_event;
  api_event.event_type = domapi::EventType::TextCompositionStart;
  std::vector<domapi::TextCompositionSpan> spans {
    domapi::TextCompositionSpan {0, 1, 1},
    domapi::TextCompositionSpan {1, 2, 2},
    domapi::TextCompositionSpan {2, 3, 3},
  };
  api_event.data.caret = 42;
  api_event.data.spans = spans;
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
  EXPECT_SCRIPT_VALID("var spans = event.spans;");
  EXPECT_SCRIPT_EQ("3", "spans.length");
  EXPECT_SCRIPT_EQ("1 2 3",
    "spans[0].data + ' ' + spans[1].data + ' ' + spans[2].data");
  EXPECT_SCRIPT_EQ("42", "event.caret");
  EXPECT_SCRIPT_EQ("foo", "event.data");
}

}  // namespace
