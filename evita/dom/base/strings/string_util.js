// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {
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
  return charCode == Unicode.SPACE || charCode == Unicode.TAB ||
      charCode == Unicode.LF || charCode == Unicode.CR;
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

/**
 * @param {number} charCode
 * @return {number}
 */
function toAsciiLowerCase(charCode) {
  if (isAsciiUpperCase(charCode))
    return charCode + 0x20;
  return charCode;
}

/**
 * @param {number} charCode
 * @return {number}
 */
function toAsciiUpperCase(charCode) {
  if (isAsciiLowerCase(charCode))
    return charCode - 0x20;
  return charCode;
}

/** @param {number} charCode @return {boolean} */
base.isAsciiAlpha = isAsciiAlpha;

/** @param {number} charCode @return {boolean} */
base.isAsciiDigit = isAsciiDigit;

/** @param {number} charCode @return {boolean} */
base.isAsciiLowerCase = isAsciiLowerCase;

/** @param {number} charCode @return {boolean} */
base.isAsciiUpperCase = isAsciiUpperCase;

/** @param {number} charCode @return {boolean} */
base.isAsciiWhitespace = isAsciiWhitespace;

/** @param {number} charCode @return {boolean} */
base.isHexDigit = isHexDigit;

/** @param {number} charCode @return {number} */
base.toAsciiLowerCase = toAsciiLowerCase;

/** @param {number} charCode @return {number} */
base.toAsciiUpperCase = toAsciiUpperCase;
});
