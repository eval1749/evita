// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('testing');

goog.scope(function() {

function testFindBracket(sample, direction) {
  const doc = new TextDocument();
  const anchor = sample.replace('|', '').indexOf('^');
  console.assert(anchor >= 0, `No ^ in sample "${sample}".`);
  const focus = sample.replace('^', '').indexOf(`|`);
  console.assert(focus >= 0, `No | in sample "${sample}".`);
  const text = sample.replace(/[|^]/g, '');
  const quotedText = text.replace(/[!]/g, '"').replace(/[#]/g, '\\');
  doc.replace(0, doc.length, quotedText);
  doc.setSyntax(0, doc.length, 'other');
  highlight(doc);
  const pos = new TextPosition(doc, anchor);
  pos.move(Unit.BRACKET, direction);
  if (pos.offset > anchor) {
    return text.substr(0, anchor) + '^' +
        text.substr(anchor, pos.offset - anchor) + '|' +
        text.substr(pos.offset);
  }
  return text.substr(0, pos.offset) + '|' +
      text.substr(pos.offset, anchor - pos.offset) + '^' + text.substr(anchor);
}

function highlight(document) {
  const pos = new TextPosition(document, 0);
  const range = new TextRange(document);
  let state = 'NORMAL';
  let color = 0;
  let charSyntax = 'other';

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
    const charCode = pos.charCode();
    switch (state) {
      case 'NORMAL':
        if (charCode === Unicode.QUOTATION_MARK) {
          finish(0);
          state = 'STRING1';
          color = 0x000080;
          charSyntax = 'one';
        } else if (charCode === Unicode.SOLIDUS) {
          finish(0);
          state = 'COMMENT_1';
          color = 0x008000;
          charSyntax = 'two';
        } else if (
            charCode === Unicode.LEFT_CURLY_BRACKET ||
            charCode === Unicode.RIGHT_CURLY_BRACKET) {
          finish(0);
          color = 0;
          charSyntax = 'three';
          finish(1);
          pos.move(Unit.CHARACTER);
        }
        break;
      case 'COMMENT_1':
        if (charCode === Unicode.SOLIDUS)
          finish(1);
        break;
      case 'STRING1':
        if (charCode === Unicode.QUOTATION_MARK)
          finish(1);
        else if (charCode === Unicode.REVERSE_SOLIDUS)
          state = 'STRING1_ESCAPE';
        break;
      case 'STRING1_ESCAPE':
        state = 'STRING1';
        break;
    }
  }
}

function testBracketBackwardTest(t, sample, description = '') {
  t.expect(testFindBracket(sample, -1), description).toEqual(sample);
}

function testFindBracketForward(t, sample, description = '') {
  t.expect(testFindBracket(sample, 1), description).toEqual(sample);
}

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

testing.test('TextPosition.findBacketForward', function(t) {
  testFindBracketForward(t, '^(foo)| (bar)');
  testFindBracketForward(t, '(^foo)| (bar)');
  testFindBracketForward(t, '(f^oo)| (bar)');
  testFindBracketForward(t, '(fo^o)| (bar)');
  testFindBracketForward(t, '(foo^)| (bar)');
  testFindBracketForward(t, '(foo)^ |(bar)');
  testFindBracketForward(t, '(foo) ^(bar)|');

  testFindBracketForward(
      t, '}^ else |{ foo', 'bracket and others have different colors.');

  testFindBracketForward(
      t, '^(foo !bar)! baz)|', 'string is a different matching context.');
  testFindBracketForward(
      t, '!^(foo! bar) !baz)|!', 'string is a different matching context.');

  testFindBracketForward(t, '^(foo #) bar)| baz', 'escape character in string');
  testFindBracketForward(
      t, '^(foo ##)| bar) baz', 'escaped escape character in string');

  testFindBracketForward(t, '^(foo|] bar', 'mismatched bracket');

  testFindBracketForward(t, '^((foo))|', 'skip nested bracket');
  testFindBracketForward(t, '(^(foo)|)', 'inner bracket pair');
});

testing.test('TextPosition.findBacketBackward', function(t) {
  testBracketBackwardTest(t, '|^(foo) (bar)');
  testBracketBackwardTest(t, '|(^foo) (bar)');
  testBracketBackwardTest(t, '|(f^oo) (bar)');
  testBracketBackwardTest(t, '|(fo^o) (bar)');
  testBracketBackwardTest(t, '|(foo^) (bar)');
  testBracketBackwardTest(t, '|(foo)^ (bar)');
  testBracketBackwardTest(t, '(foo)| ^(bar)');

  testBracketBackwardTest(
      t, '}| else ^{ foo', 'bracket and others have different colors.');

  testBracketBackwardTest(
      t, '|(foo !(bar! baz)^', 'string is a different matching context.');

  // escape
  testBracketBackwardTest(t, '|(foo #( bar)^ baz');
  testBracketBackwardTest(t, '(foo ##|( bar)^ baz');

  testBracketBackwardTest(
      t, '(foo]|^ bar', "We don't move caret for mismatched right bracket.");
  testBracketBackwardTest(
      t, '!|(foo! (bar !baz)^!',
      "We don't care mismatched left bracket in different character syntax.");

  testBracketBackwardTest(t, '|^((foo))', 'nested brackets');
  testBracketBackwardTest(t, '|(^(foo))', 'nested brackets');
  testBracketBackwardTest(t, '(|(^foo))', 'nested brackets');
  testBracketBackwardTest(t, '(|(f^oo))', 'nested brackets');
  testBracketBackwardTest(t, '(|(f^oo))', 'nested brackets');
  testBracketBackwardTest(t, '(|(fo^o))', 'nested brackets');
  testBracketBackwardTest(t, '(|(foo^))', 'nested brackets');
  testBracketBackwardTest(t, '(|(foo^))', 'nested brackets');
  testBracketBackwardTest(t, '(|(foo)^)', 'nested brackets');
  testBracketBackwardTest(t, '|((foo))^', 'nested brackets');
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
