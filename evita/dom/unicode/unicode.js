// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('unicode');

goog.scope(function() {

// TODO(eval1749): We should introduce following functions:
//  unicode.scriptOf(charCode)

/**
 * @param {number} charCode
 * @return {string}
 */
function categoryOf(charCode) {
  return Unicode.UCD[charCode].category;
}

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isLetter(charCode) {
  return categoryOf(charCode).charCodeAt(0) === 0x4C;
}

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isLowerCase(charCode) {
  return categoryOf(charCode) === 'Ll';
}

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isSeparator(charCode) {
  return categoryOf(charCode).charCodeAt(0) === 0x5A;
}

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isTitleCase(charCode) {
  return categoryOf(charCode) === 'Lt';
}

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isUpperCase(charCode) {
  return categoryOf(charCode) === 'Lu';
}

/**
 * @param {number} charCode
 * @return {!Unicode.Script}
 */
function scriptOf(charCode) {
  return Unicode.UCD[charCode].script;
}

/**
 * @param {number} charCode
 * @return {string}
 */
unicode.categoryOf = categoryOf;

/**
 * @param {number} charCode
 * @return {boolean}
 */
unicode.isLetter = isLetter;

/**
 * @param {number} charCode
 * @return {boolean}
 */
unicode.isLowerCase = isLowerCase;

/**
 * @param {number} charCode
 * @return {boolean}
 */
unicode.isSeparator = isSeparator;

/**
 * @param {number} charCode
 * @return {boolean}
 */
unicode.isTitleCase = isTitleCase;

/**
 * @param {number} charCode
 * @return {boolean}
 */
unicode.isUpperCase = isUpperCase;

/**
 * @param {number} charCode
 * @return {!Unicode.Script}
 */
unicode.scriptOf = scriptOf;
});
