// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('testing');

goog.scope(function() {

function testFindBracket(sample, direction) {
  const doc = new TextDocument();
  const anchor = sample.replace('|', '').indexOf('^');
  console.assert(anchor >= 0, `No ^ in sample "${sample}".`);
  const expected = sample.replace('^', '').indexOf('|');
  console.assert(expected >= 0, `No | in sample "${sample}".`);
  const text = sample.replace(/[|^]/g, '');
  const quotedText = text.replace(/[!]/g, '"').replace(/[#]/g, '\\');
  doc.replace(0, doc.length, quotedText);
  doc.setSyntax(0, doc.length, 'other');
  highlight(doc);
  const actual = doc.computeMotion_(Unit.BRACKET, direction, anchor);
  if (typeof(actual) !== 'number')
    return actual.toString();
  if (actual > anchor) {
    return text.substr(0, anchor) + '^' +
        text.substr(anchor, actual - anchor) + '|' +
        text.substr(actual);
  }
  return text.substr(0, actual) + '|' +
      text.substr(actual, anchor - actual) + '^' + text.substr(anchor);
}

function highlight(document) {
  let state = 'NORMAL';
  let charSyntax = 'other';
  let start = 0;

  function finish(offset, amount) {
    const end = offset + amount;
    if (start != end)
      document.setSyntax(start, end, charSyntax);
    state = 'NORMAL';
    charSyntax = 'other';
    start = end;
  }

  for (let offset = 0; offset < document.length; ++offset) {
    const charCode = document.charCodeAt(offset);
    switch (state) {
      case 'NORMAL':
        if (charCode === Unicode.QUOTATION_MARK) {
          finish(offset, 0);
          state = 'STRING1';
          charSyntax = 'one';
        } else if (charCode === Unicode.SOLIDUS) {
          finish(offset, 0);
          state = 'COMMENT_1';
          charSyntax = 'two';
        }
        break;
      case 'COMMENT_1':
        if (charCode === Unicode.SOLIDUS)
          finish(offset, 1);
        break;
      case 'STRING1':
        if (charCode === Unicode.QUOTATION_MARK)
          finish(offset, 1);
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

  testFindBracketForward(
      t, '|^(foo] bar',
      'we should stop mismatched bracket to know mismatched bracket.');
  testFindBracketForward(
      t, '^{ if |(foo }',
      'we should stop mismatched bracket to know mismatched bracket.');
  testFindBracketForward(
      t, '{ if |^(foo }',
      'we should stop mismatched bracket to know mismatched bracket.');

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


testing.test('TextDocument.replace', function(t) {
  const doc = new TextDocument();
  doc.replace(0, 0, 'abc');
  t.expect(doc.slice(0), 'insert to empty doc').toEqual('abc');

  doc.replace(3, 3, 'def');
  t.expect(doc.slice(0), 'append to end').toEqual('abcdef');

  doc.replace(2, 4, 'CD');
  t.expect(doc.slice(0), 'replace with same length').toEqual('abCDef');

  doc.replace(2, 4, '');
  t.expect(doc.slice(0), 'delete').toEqual('abef');

  doc.replace(1, 1, 'XY');
  t.expect(doc.slice(0), 'insert').toEqual('aXYbef');

  doc.replace(1, 6, 'xyz');
  t.expect(doc.slice(0), 'replace with shorter').toEqual('axyz');

  doc.replace(1, 3, '012');
  t.expect(doc.slice(0), 'replace with longer').toEqual('a012z');
});

});
