// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

namespace dom {

class RegExpTest : public AbstractDomTest {
 protected:
  RegExpTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(RegExpTest);
};

TEST_F(RegExpTest, execOnDocument_) {
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('foo');"
      "var range = new Range(doc);"
      "range.text = 'foo bar baz';"
      "function exec(source, match_exact) {"
      "  var regexp = new Editor.RegExp(source, {matchExact: match_exact});"
      "  var matches = doc.match_(regexp, 0, doc.length);"
      "  if (!matches)"
      "    return null;"
      "  return matches.map(function(match) {"
      "     var range = new Range(doc, match.start, match.end);"
      "     return range.text;"
      "  }).join(',');"
      "}");
  EXPECT_SCRIPT_EQ("bar", "exec('bar', true)");
  EXPECT_SCRIPT_EQ("bar baz,ar ba", "exec('b(.+)z', false)");
}

}  // namespace dom
