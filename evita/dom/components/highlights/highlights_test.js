// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

function scan(machine, sample) {
  machine.resetTo(0);
  const result = [];
  let lastState = -1;
  let lengthOfState = 0;
  for (const char of sample) {
    const charCode = char.charCodeAt(0);
    const state = machine.updateState(charCode);
    if (lastState === -1) {
      lastState = state;
      lengthOfState = 1;
      continue;
    }
    if (lastState === state) {
      ++lengthOfState;
      continue;
    }
    const syntax = machine.syntaxOf(lastState);
    result.push(`${syntax}:${lengthOfState}`);
    // When |state| is zero, the last state is an acceptable state and
    // no more consumes input. Thus, we need to compute new state with
    // current input.
    lastState = state !== 0 ? state : machine.updateState(charCode);
    lengthOfState = 1;
  }
  if (lengthOfState > 0) {
    const syntax = machine.syntaxOf(lastState);
    result.push(`${syntax}:${lengthOfState}`);
  }
  return result;
}

const namespace = highlights;
namespace.scan = scan;
});

testing.test('CppTokenStateMachine', function(t) {
  const machine = new highlights.CppTokenStateMachine();
  const scan = highlights.scan.bind(this, machine);
  t.expect(scan('/* abc */ def "ghi" \'jkl\'')).toEqual([
    'operator:1', 'comment:6', 'comment:1', 'comment:1', 'whitespace:1',
    'identifier:3', 'whitespace:1', 'string_literal:4', 'string_literal:1',
    'whitespace:1', 'string_literal:4', 'string_literal:1'
  ]);
  t.expect(scan('#if')).toEqual(['keyword:1', 'identifier:2']);
  t.expect(scan('/**/')).toEqual([
    'operator:1', 'comment:1', 'comment:1', 'comment:1'
  ]);
  t.expect(scan('\x2F/*')).toEqual(['operator:1', 'comment:2']);
  t.expect(
       scan('\x2F/ xyz\\\nbar'),
       'Backslash before newline continues line comment to next line')
      .toEqual(['operator:1', 'comment:5', 'comment:1', 'comment:4']);
});
