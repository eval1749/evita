// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('testing');

goog.scope(function() {

testing.test('TextPosition.basic', function(t) {
  const doc = new TextDocument();
  doc.replace(0, doc.length, 'foo bar baz');
  const pos = new TextPosition(doc, 2);

  t.expect(pos.document).toEqual(doc);
  t.expect(pos.charCode()).toEqual(111);
  try {
    new TextPosition(doc, -1);
  } catch (error) {
    t.expect(error.toString())
        .toEqual('RangeError: Invalid offset -1 for [object TextDocument]');
  }
  try {
    new TextPosition(doc, 100);
  } catch (error) {
    t.expect(error.toString())
        .toEqual('RangeError: Invalid offset 100 for [object TextDocument]');
  }
});

testing.test('TextPosition.move', function(t) {
  const doc = new TextDocument();
  doc.replace(0, doc.length, 'foo bar baz');
  const pos = new TextPosition(doc, 0);

  t.expect(pos.move(Unit.CHARACTER).offset).toEqual(1);
  t.expect(pos.move(Unit.CHARACTER, 2).offset).toEqual(3);
  t.expect(pos.move(Unit.CHARACTER, -1).offset).toEqual(2);
});

testing.test('TextPosition.moveBracket', function(t) {
  const doc = new TextDocument();
  const range = new TextRange(doc);

  function testIt(sample, direction) {
    range.start = 0;
    range.end = doc.length;
    var text =
        sample.replace('^', '').replace(/[|]/g, '"').replace(/[#]/g, '\\');
    range.text = text;
    range.setSyntax('other');
    highlight(doc);
    var pos = new TextPosition(doc, sample.indexOf('^'));
    pos.move(Unit.BRACKET, direction);
    var text2 = text.replace(/\x22/g, '|').replace(/[\\]/g, '#');
    return text2.substr(0, pos.offset) + '^' + text2.substr(pos.offset);
  }

  function backward(sample) { return testIt(sample, -1); }

  function forward(sample) { return testIt(sample, 1); }

  function highlight(document) {
    var pos = new TextPosition(document, 0);
    var range = new TextRange(document);
    var state = 'NORMAL';
    var color = 0;
    var charSyntax = 'other';

    function finish(advance) {
      range.end = pos.offset + advance;
      if (!range.collapsed) {
        range.setStyle({color: color});
        range.setSyntax(charSyntax);
      }
      range.collapseTo(range.end);
      state = 'NORMAL';
      color = 0;
      charSyntax = 'other';
    }

    for (; pos.offset < document.length; pos.move(Unit.CHARACTER)) {
      var char_code = pos.charCode();
      switch (state) {
        case 'NORMAL':
          if (char_code == Unicode.QUOTATION_MARK) {
            finish(0);
            state = 'STRING1';
            color = 0x000080;
            charSyntax = 'one';
          } else if (char_code == Unicode.SOLIDUS) {
            finish(0);
            state = 'COMMENT_1';
            color = 0x008000;
            charSyntax = 'two';
          } else if (
              char_code == Unicode.LEFT_CURLY_BRACKET ||
              char_code == Unicode.RIGHT_CURLY_BRACKET) {
            finish(0);
            color = 0;
            charSyntax = 'three';
            finish(1);
            pos.move(Unit.CHARACTER);
          }
          break;
        case 'COMMENT_1':
          if (char_code == Unicode.SOLIDUS)
            finish(1);
          break;
        case 'STRING1':
          if (char_code == Unicode.QUOTATION_MARK)
            finish(1);
          else if (char_code == Unicode.REVERSE_SOLIDUS)
            state = 'STRING1_ESCAPE';
          break;
        case 'STRING1_ESCAPE':
          state = 'STRING1';
          break;
      }
    }
  }
  // forward
  t.expect(forward('^(foo) (bar)')).toEqual("(foo)^ (bar)");
  t.expect(forward('(^foo) (bar)')).toEqual("(foo)^ (bar)");
  t.expect(forward('(f^oo) (bar)')).toEqual("(foo)^ (bar)");
  t.expect(forward('(fo^o) (bar)')).toEqual("(foo)^ (bar)");
  t.expect(forward('(foo^) (bar)')).toEqual("(foo)^ (bar)");
  t.expect(forward('(foo)^ (bar)')).toEqual("(foo) ^(bar)");
  t.expect(forward('(foo) ^(bar)')).toEqual("(foo) (bar)^");

  // bracket and others have different colors.
  t.expect(forward('}^ else { foo')).toEqual("} else ^{ foo");

  // string is a different matching context.
  t.expect(forward('^(foo |bar)| baz)')).toEqual("(foo |bar)| baz)^");
  t.expect(forward('|^(foo| bar) |baz)|')).toEqual("|(foo| bar) |baz)^|");

  // escape
  t.expect(forward('^(foo #) bar) baz')).toEqual("(foo #) bar)^ baz");
  t.expect(forward('^(foo ##) bar) baz')).toEqual("(foo ##)^ bar) baz");

  // mismatched
  t.expect(forward('^(foo] bar')).toEqual("(foo^] bar");

  // nested parenthesis
  t.expect(forward('^((foo))')).toEqual("((foo))^");
  t.expect(forward('(^(foo))')).toEqual("((foo)^)");

  // backward
  t.expect(backward('^(foo) (bar)')).toEqual("^(foo) (bar)");
  t.expect(backward('(^foo) (bar)')).toEqual("^(foo) (bar)");
  t.expect(backward('(f^oo) (bar)')).toEqual("^(foo) (bar)");
  t.expect(backward('(fo^o) (bar)')).toEqual("^(foo) (bar)");
  t.expect(backward('(foo^) (bar)')).toEqual("^(foo) (bar)");
  t.expect(backward('(foo)^ (bar)')).toEqual("^(foo) (bar)");
  t.expect(backward('(foo) ^(bar)')).toEqual("(foo)^ (bar)");

  // bracket and others have different colors.
  t.expect(backward('} else ^{ foo')).toEqual("}^ else { foo");

  // string is a different matching context.
  t.expect(backward('(foo |(bar| baz)^')).toEqual("^(foo |(bar| baz)");

  // escape
  t.expect(backward('(foo #( bar)^ baz')).toEqual("^(foo #( bar) baz");
  t.expect(backward('(foo ##( bar)^ baz')).toEqual("(foo ##^( bar) baz");

  // mismatched
  t.expect(
       backward('(foo]^ bar'),
       "We don't move caret for mismatched right bracket.")
      .toEqual("(foo]^ bar");

  t.expect(
       backward('|(foo| (bar |baz)^|'),
       "We don't care mismatched left bracket in different character syntax.")

      .toEqual("|^(foo| (bar |baz)|", "backward('|(foo| (bar |baz)^|')");

  // nested parenthesis
  t.expect(backward('^((foo))')).toEqual("^((foo))");
  t.expect(backward('(^(foo))')).toEqual("^((foo))");
  t.expect(backward('((^foo))')).toEqual("(^(foo))");
  t.expect(backward('((f^oo))')).toEqual("(^(foo))");
  t.expect(backward('((f^oo))')).toEqual("(^(foo))");
  t.expect(backward('((fo^o))')).toEqual("(^(foo))");
  t.expect(backward('((foo^))')).toEqual("(^(foo))");
  t.expect(backward('((foo^))')).toEqual("(^(foo))");
  t.expect(backward('((foo)^)')).toEqual("(^(foo))");
  t.expect(backward('((foo))^')).toEqual("^((foo))");
});

testing.test('TextPosition.moveWhile', function(t) {
  function whitespace() {
    const charCode = this.charCode();
    return unicode.isSeparator(charCode);
  }

  const doc = new TextDocument();
  doc.replace(0, doc.length, '   foo bar');
  const pos = new TextPosition(doc, 0);

  t.expect(pos.moveWhile(whitespace).offset).toEqual(1);
  t.expect(pos.moveWhile(whitespace, -1).offset).toEqual(0);
  t.expect(pos.moveWhile(whitespace, 2).offset).toEqual(2);
  t.expect(pos.moveWhile(whitespace, -1).offset).toEqual(1);
  t.expect(pos.moveWhile(whitespace, Count.FORWARD).offset).toEqual(3);
  t.expect(pos.moveWhile(whitespace, Count.BACKWARD).offset).toEqual(0);
});

});
