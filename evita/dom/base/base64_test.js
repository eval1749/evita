// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('Base64', function(t) {
  function makeUint8Vector(input) {
    const output = new Uint8Array(input.length);
    let index = 0;
    for (const char of input) {
      output[index] = char.charCodeAt(0);
      ++index;
    }
    return output;
  }

  function roundTrip(input) {
    const base64 = base.btoa(makeUint8Vector(input));
    const base10 = base.atob(base64);
    const output = [];
    for (const charCode of base10)
      output.push(String.fromCharCode(charCode));
    return output.join('');
  }

  t.expect(roundTrip('a')).toEqual('a');
  t.expect(roundTrip('ab')).toEqual('ab');
  t.expect(roundTrip('abc')).toEqual('abc');
  t.expect(roundTrip('abcd')).toEqual('abcd');
  t.expect(roundTrip('abcde')).toEqual('abcde');
  t.expect(roundTrip('abcdef')).toEqual('abcdef');
});
