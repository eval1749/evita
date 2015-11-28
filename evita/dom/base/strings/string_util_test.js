// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('base.isAsciiAlpha', function(t) {
  const alphas = new Set();
  Array.from("ABCDEFGHIJKLMNOPQRSTUVWXYZ").forEach(char => {
    alphas.add(char.charCodeAt(0));
  });
  Array.from("abcdefghijklmnopqrstuvwxyz").forEach(char => {
    alphas.add(char.charCodeAt(0));
  });
  for (let charCode = 0; charCode < 256; ++charCode) {
    const description = `base.isAsciiAlpha(${charCode})`;
    t.expect(base.isAsciiAlpha(charCode), description)
        .toEqual(alphas.has(charCode));
  }
});

testing.test('base.isAsciiDigit', function(t) {
  // TODO(eval1749): Due to limitation of closure compiler, we can't use
  // second parameter of |Array.from(arrayLike, mapFn?, thisArg?)|.
  const digits = new Set(
      Array.from("0123456789").map(char => char.charCodeAt(0)));
  for (let charCode = 0; charCode < 256; ++charCode) {
    const description = `base.isAsciiDigit(${charCode})`;
    t.expect(base.isAsciiDigit(charCode), description)
        .toEqual(digits.has(charCode));
  }
});

testing.test('base.isAsciiLowerCase', function(t) {
  // TODO(eval1749): Due to limitation of closure compiler, we can't use
  // second parameter of |Array.from(arrayLike, mapFn?, thisArg?)|.
  const lowerCases = new Set(
      Array.from("abcdefghijklmnopqrstuvwxyz").map(char => char.charCodeAt(0)));
  for (let charCode = 0; charCode < 256; ++charCode) {
    const description = `base.isAsciiLowerCase(${charCode})`;
    t.expect(base.isAsciiLowerCase(charCode), description)
        .toEqual(lowerCases.has(charCode));
  }
});

testing.test('base.isAsciiUpperCase', function(t) {
  // TODO(eval1749): Due to limitation of closure compiler, we can't use
  // second parameter of |Array.from(arrayLike, mapFn?, thisArg?)|.
  const upperCases = new Set(
      Array.from("ABCDEFGHIJKLMNOPQRSTUVWXYZ").map(char => char.charCodeAt(0)));
  for (let charCode = 0; charCode < 256; ++charCode) {
    const description = `base.isAsciiUpperCase(${charCode})`;
    t.expect(base.isAsciiUpperCase(charCode), description)
        .toEqual(upperCases.has(charCode));
  }
});

testing.test('base.isHexDigit', function(t) {
  // TODO(eval1749): Due to limitation of closure compiler, we can't use
  // second parameter of |Array.from(arrayLike, mapFn?, thisArg?)|.
  const hexDigits = new Set(
      Array.from("0123456789ABCDEFabcdef").map(char => char.charCodeAt(0)));
  for (let charCode = 0; charCode < 256; ++charCode) {
    const description = `base.isHexDigit(${charCode})`;
    t.expect(base.isHexDigit(charCode), description)
        .toEqual(hexDigits.has(charCode));
  }
});
