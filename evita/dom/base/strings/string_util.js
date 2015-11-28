// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'base', function($export) {
  /**
   * @param {number} charCode
   * @return {boolean}
   */
  function isAsciiAlpha(charCode) {
    return isAsciiUpperCase(charCode) || isAsciiLowerCase(charCode);
  }

  /**
   * @param {number} charCode
   * @return {boolean}
   */
  function isAsciiDigit(charCode) {
    return charCode >= Unicode.DIGIT_ZERO && charCode <= Unicode.DIGIT_NINE;
  }

  /**
   * @param {number} charCode
   * @return {boolean}
   */
  function isAsciiLowerCase(charCode) {
    return charCode >= Unicode.LATIN_SMALL_LETTER_A &&
           charCode <= Unicode.LATIN_SMALL_LETTER_Z;
  }

  /**
   * @param {number} charCode
   * @return {boolean}
   */
  function isAsciiUpperCase(charCode) {
    return charCode >= Unicode.LATIN_CAPITAL_LETTER_A &&
           charCode <= Unicode.LATIN_CAPITAL_LETTER_Z;
  }

  /**
   * @param {number} charCode
   * @return {boolean}
   */
  function isAsciiWhitespace(charCode) {
    return charCode == Unicode.SPACE ||
           charCode == Unicode.TAB ||
           charCode == Unicode.LF ||
           charCode == Unicode.CR;
  }

  /**
   * @param {number} charCode
   * @return {boolean}
   */
  function isHexDigit(charCode) {
    if (charCode >= Unicode.DIGIT_ZERO && charCode <= Unicode.DIGIT_NINE)
      return true;
    if (charCode >= Unicode.LATIN_CAPITAL_LETTER_A &&
        charCode <= Unicode.LATIN_CAPITAL_LETTER_F) {
      return true;
    }
    if (charCode >= Unicode.LATIN_SMALL_LETTER_A &&
        charCode <= Unicode.LATIN_SMALL_LETTER_F) {
      return true;
    }
    return false;
  }

  $export({isAsciiAlpha, isAsciiDigit, isAsciiLowerCase, isAsciiUpperCase,
           isAsciiWhitespace, isHexDigit});
});
