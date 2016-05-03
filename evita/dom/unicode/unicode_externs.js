// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('unicode');

/**
 * @param {number} charCode
 * @return {string}
 */
unicode.categoryOf = function(charCode) {};

/**
 * @param {number} charCode
 * @return {boolean}
 */
unicode.isLetter = function(charCode) {};

/**
 * @param {number} charCode
 * @return {boolean}
 */
unicode.isLowerCase = function(charCode) {};

/**
 * @param {number} charCode
 * @return {boolean}
 */
unicode.isSeparator = function(charCode) {};

/**
 * @param {number} charCode
 * @return {boolean}
 */
unicode.isTitleCase = function(charCode) {};

/**
 * @param {number} charCode
 * @return {boolean}
 */
unicode.isUpperCase = function(charCode) {};

/**
 * @param {number} charCode
 * @return {!Unicode.Script}
 */
unicode.scriptOf = function(charCode) {};
