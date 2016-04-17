// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

function scan(machine, sample) {
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
namespace.scan = scan;
});

testing.test('CppStateRangeStateMachine', function(t) {
  const machine = new highlights.CppTokenStateMachine();
  const scan = highlights.scan.bind(this, machine);
  t.expect(scan('/* abc */ def')).toEqual('c9 w1 i3');
  t.expect(scan('"abc"')).toEqual('s5');
  t.expect(scan('\'abc\'')).toEqual('s5');
  t.expect(scan('#if')).toEqual('k1 i2');
  t.expect(scan('/**/')).toEqual('c4');
  t.expect(scan('\x2F/*')).toEqual('c3');
  t.expect(
       scan('\x2F/ xyz\\\nbar'),
       'Backslash before newline continues line comment to next line')
      .toEqual('c11');
});
