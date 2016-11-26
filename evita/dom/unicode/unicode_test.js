// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require("testing");
goog.require("unicode");

goog.scope(function() {

testing.test('Unicode.CATEGORY_DATA', function(t) {
  t.expect(Unicode.CATEGORY_DATA.length).toEqual(0x10FFFF + 1);
  t.expect(Unicode.CATEGORY_DATA['0'.charCodeAt(0)]).toEqual(9);
  t.expect(Unicode.CATEGORY_DATA['!'.charCodeAt(0)]).toEqual(23);
  t.expect(Unicode.CATEGORY_DATA['A'.charCodeAt(0)]).toEqual(1);
  t.expect(Unicode.CATEGORY_DATA['Z'.charCodeAt(0)]).toEqual(1);
  t.expect(Unicode.CATEGORY_DATA['a'.charCodeAt(0)]).toEqual(2);
  t.expect(Unicode.CATEGORY_DATA['z'.charCodeAt(0)]).toEqual(2);
  t.expect(Unicode.CATEGORY_DATA[0x1F2]).toEqual(3);
  t.expect(Unicode.CATEGORY_DATA[0x3042]).toEqual(5);
});

testing.test('Unicode.CATEGORY_SHORT_NAMES', function(t) {
  t.expect(Unicode.CATEGORY_SHORT_NAMES.length).toEqual(30);
});

testing.test('Unicode.SCRIPT_DATA', function(t) {
  t.expect(Unicode.SCRIPT_DATA.length).toEqual(0x10FFFF + 1);
  t.expect(Unicode.SCRIPT_DATA['0'.charCodeAt(0)])
      .toEqual(Unicode.Script.COMMON);
  t.expect(Unicode.SCRIPT_DATA['!'.charCodeAt(0)])
      .toEqual(Unicode.Script.COMMON);
  t.expect(Unicode.SCRIPT_DATA['A'.charCodeAt(0)])
      .toEqual(Unicode.Script.LATIN);
  t.expect(Unicode.SCRIPT_DATA['Z'.charCodeAt(0)])
      .toEqual(Unicode.Script.LATIN);
  t.expect(Unicode.SCRIPT_DATA['a'.charCodeAt(0)])
      .toEqual(Unicode.Script.LATIN);
  t.expect(Unicode.SCRIPT_DATA['z'.charCodeAt(0)])
      .toEqual(Unicode.Script.LATIN);
  t.expect(Unicode.SCRIPT_DATA[0x1F2]).toEqual(Unicode.Script.LATIN);
  t.expect(Unicode.SCRIPT_DATA[0x3042]).toEqual(Unicode.Script.HIRAGANA);
});

// See "evita/dom/unicode/unicode_icu.cc" for implementation
testing.test('Unicode.SCRIPT_NAMES', function(t) {
  t.expect(Unicode.SCRIPT_NAMES[100]).toEqual('VISIBLE_SPEECH');
  t.expect(Unicode.SCRIPT_NAMES[166]).toEqual('SIDDHAM');
  t.expect(Unicode.SCRIPT_NAMES[174]).toEqual('SYMBOLS_EMOJI');
  t.expect(Unicode.SCRIPT_NAMES.length).toEqual(175);
});

testing.test('unicode.categoryOf', function(t) {
  t.expect(unicode.categoryOf('0'.charCodeAt(0))).toEqual(Unicode.Category.Nd);
  t.expect(unicode.categoryOf('!'.charCodeAt(0))).toEqual(Unicode.Category.Po);
  t.expect(unicode.categoryOf('A'.charCodeAt(0))).toEqual(Unicode.Category.Lu);
  t.expect(unicode.categoryOf('Z'.charCodeAt(0))).toEqual(Unicode.Category.Lu);
  t.expect(unicode.categoryOf('a'.charCodeAt(0))).toEqual(Unicode.Category.Ll);
  t.expect(unicode.categoryOf('z'.charCodeAt(0))).toEqual(Unicode.Category.Ll);
  t.expect(unicode.categoryOf(0x1F2)).toEqual(Unicode.Category.Lt);
});

testing.test('unicode.isLetter', function(t) {
  t.expect(unicode.isLetter('0'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isLetter('!'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isLetter('A'.charCodeAt(0))).toEqual(true);
  t.expect(unicode.isLetter('Z'.charCodeAt(0))).toEqual(true);
  t.expect(unicode.isLetter('a'.charCodeAt(0))).toEqual(true);
  t.expect(unicode.isLetter('z'.charCodeAt(0))).toEqual(true);
});

testing.test('unicode.isLowerCase', function(t) {
  t.expect(unicode.isLowerCase('0'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isLowerCase('!'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isLowerCase('A'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isLowerCase('Z'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isLowerCase('a'.charCodeAt(0))).toEqual(true);
  t.expect(unicode.isLowerCase('z'.charCodeAt(0))).toEqual(true);
});

testing.test('unicode.isTitleCase', function(t) {
  t.expect(unicode.isTitleCase('0'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isTitleCase('!'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isTitleCase('A'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isTitleCase('Z'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isTitleCase('a'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isTitleCase('z'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isTitleCase(0x1F2)).toEqual(true);
});

testing.test('unicode.isUpperCase', function(t) {
  t.expect(unicode.isUpperCase('0'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isUpperCase('!'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isUpperCase('A'.charCodeAt(0))).toEqual(true);
  t.expect(unicode.isUpperCase('Z'.charCodeAt(0))).toEqual(true);
  t.expect(unicode.isUpperCase('a'.charCodeAt(0))).toEqual(false);
  t.expect(unicode.isUpperCase('z'.charCodeAt(0))).toEqual(false);
});

testing.test('unicode.scriptOf', function(t) {
  t.expect(unicode.scriptOf('0'.charCodeAt(0))).toEqual(Unicode.Script.COMMON);
  t.expect(unicode.scriptOf('!'.charCodeAt(0))).toEqual(Unicode.Script.COMMON);
  t.expect(unicode.scriptOf('A'.charCodeAt(0))).toEqual(Unicode.Script.LATIN);
  t.expect(unicode.scriptOf('Z'.charCodeAt(0))).toEqual(Unicode.Script.LATIN);
  t.expect(unicode.scriptOf('a'.charCodeAt(0))).toEqual(Unicode.Script.LATIN);
  t.expect(unicode.scriptOf('z'.charCodeAt(0))).toEqual(Unicode.Script.LATIN);
  t.expect(unicode.scriptOf(0x1F2)).toEqual(Unicode.Script.LATIN);
  t.expect(unicode.scriptOf(0x3042)).toEqual(Unicode.Script.HIRAGANA);
});

});
