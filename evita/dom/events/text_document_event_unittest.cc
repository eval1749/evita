// Copyright (c) 2013 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"
#include "evita/dom/testing/mock_view_impl.h"
#include "gmock/gmock.h"

namespace dom {

using ::testing::_;

class TextDocumentEventTest : public AbstractDomTest {
 public:
  ~TextDocumentEventTest() override = default;

 protected:
  TextDocumentEventTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(TextDocumentEventTest);
};

TEST_F(TextDocumentEventTest, ctor) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_));
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "var window = new TextWindow(new TextRange(doc));"
      "var event = new TextDocumentEvent('foo', {"
      "  view: window,"
      "});");
  EXPECT_SCRIPT_FALSE("event.bubbles") << "default |bubbles| is false";
  EXPECT_SCRIPT_FALSE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.currentTarget == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("Event.PhaseType.NONE === event.eventPhase");
  EXPECT_SCRIPT_EQ("0", "event.timeStamp");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("foo", "event.type");

  // TextDocumentEvent
  EXPECT_SCRIPT_TRUE("event.view == window");
}

}  // namespace dom
