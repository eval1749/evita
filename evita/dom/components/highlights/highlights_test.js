// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

function testPaint(painterCreator, stateMachine, text) {
  const document = new TextDocument();
  const painter = painterCreator.call(this, document);
  const tokenizer =
      new highlights.base.Tokenizer(document, painter, stateMachine);
  const range = new TextRange(document);
  range.text = text;
  tokenizer.doColor(document.length);
  const result = [];
  let tokenSyntax = '';
  let tokenLength = 0;
  for (let offset = 0; offset < document.length; ++offset) {
    const syntax = document.syntaxAt(offset);
    if (tokenSyntax === '') {
      tokenSyntax = syntax;
      tokenLength = 1;
      continue;
    }
    if (tokenSyntax !== syntax) {
      result.push(`${tokenSyntax[0]}${tokenLength}`);
      tokenSyntax = syntax;
      tokenLength = 1;
      continue;
    }
    ++tokenLength;
  }
  if (tokenSyntax !== '')
    result.push(`${tokenSyntax[0]}${tokenLength}`);
  return result.join(' ');
}

function testScan(machine, sample) {
  const stateRanges = [];
  function endStateRange(stateRange, offset) {
    stateRange.end = offset;
    stateRange.syntax = machine.syntaxOf(stateRange.state);
    stateRanges.push(stateRange);
  }

  function updateState(charCode) {
    const state = machine.updateState(charCode);
    if (state !== 0)
      return state;
    // When |state| is zero, the last state is an acceptable state and
    // no more consumes input. Thus, we need to compute new state with
    // current input.
    return machine.updateState(charCode);
  }

  machine.resetTo(0);
  machine.updateState(Unicode.LF);
  let stateRange = null;
  let offset = 0;
  for (const char of sample) {
    const lastState = machine.state;
    const charCode = char.charCodeAt(0);
    const state = updateState(charCode);
    if (state === lastState && stateRange) {
      ++offset;
      continue;
    }
    if (stateRange)
      endStateRange(stateRange, offset);
    if (machine.isAcceptable(state))
      machine.resetTo(0);
    stateRange = {start: offset, end: offset + 1, state: state};
    ++offset;
  }
  endStateRange(stateRange, offset);

  return tokenize(stateRanges)
      .map(token => `${token.code}${token.end - token.start}`)
      .join(' ');
}

function tokenize(ranges) {
  const tokens = [];
  for (const range of ranges) {
    if (tokens.length === 0) {
      tokens.push({
        code: range.syntax[0],
        end: range.end,
        start: range.start,
        syntax: range.syntax,
      });
      continue;
    }
    const token = tokens[tokens.length - 1];
    if (token.syntax === '') {
      token.syntax = range.syntax;
      token.end = range.end;
      continue;
    }
    if (token.syntax === range.syntax) {
      token.end = range.end;
      continue;
    }
    tokens.push({
      code: range.syntax[0],
      end: range.end,
      start: range.start,
      syntax: range.syntax,
    });
    continue;
  }
  return tokens;
}

const namespace = highlights;
namespace.testPaint = testPaint;
namespace.testScan = testScan;
});

testing.test('CppStateRangeStateMachine', function(t) {
  const machine = new highlights.CppTokenStateMachine();
  const scan = highlights.testScan.bind(this, machine);
  t.expect(scan('/* abc */ def')).toEqual('c9 w1 i3');
  t.expect(scan('"abc"')).toEqual('s5');
  t.expect(scan('\'abc\'')).toEqual('s5');
  t.expect(scan('#if')).toEqual('i3');
  t.expect(scan('  #  if'), 'Leading whitespace is tokenized into "whitespace"')
      .toEqual('w2 i5');
  t.expect(scan('/**/')).toEqual('c4');
  t.expect(scan('\x2F/*')).toEqual('c3');
  t.expect(
       scan('\x2F/ xyz\\\nbar'),
       'Backslash before newline continues line comment to next line')
      .toEqual('c11');
});

testing.test('CppPainter', function(t) {
  const machine = new highlights.CppTokenStateMachine();
  const paint =
      highlights.testPaint.bind(this, highlights.CppPainter.create, machine);
  t.expect(paint('default:')).toEqual('k7 o1');
  t.expect(paint('foo:')).toEqual('l3 o1');
  t.expect(paint('foo::')).toEqual('i5');
  t.expect(paint('foo::bar')).toEqual('i8');
  t.expect(paint('foo::bar:')).toEqual('l8 o1');
  t.expect(paint('foo::bar::')).toEqual('i10');
});
