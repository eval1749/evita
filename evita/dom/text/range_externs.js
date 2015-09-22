// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview OldRange
 * See types_externs.js for OldRange constructor.
 * @externs
 */

/**
 * @constructor
 * @param {!Document|!Range} document_or_range
 * @param {number=} opt_start
 * @param {number=} opt_end
 */
var OldRange = function(document_or_range, opt_start, opt_end) {};

/** @enum {!symbol} */
OldRange.Case = {
  // "This is capitalized."
  CAPITALIZED_TEXT: 'CAPITALIZED_TEXT',
  // "This Is Capitalized Words."
  CAPITALIZED_WORDS: 'CAPITALIZED_WORDS',
  // "this is lower."
  LOWER: 'LOWER',
  // "ThisIsMixed."
  MIXED: 'MIXED',
  // "THIS IS UPPER."
  UPPER: 'UPPER',
};

/** @export @type {!Document} */
OldRange.prototype.document;

/** @export @type {!TextOffset} */
OldRange.prototype.end = 0;

/** @export @type {number} */
OldRange.prototype.length;

/** @export @type {!TextOffset} */
OldRange.prototype.start = 0;

/** @export @type {!string} */
OldRange.prototype.text = '';

/** @return {!OldRange} */
OldRange.prototype.capitalize = function() {}

/** @type {boolean} */
OldRange.prototype.collapsed;

/**
 * @return {!OldRange.Case}
 */
OldRange.prototype.analyzeCase = function() {};

/**
 * @param {!TextOffset} position
 */
OldRange.prototype.collapseTo = function(position) {};

/**
 * @param {Unit} unit
 * @param {number=} opt_count, default is one.
 * @return {!OldRange}
 */
OldRange.prototype.delete = function(unit, opt_count) {};

/**
 * @param {Unit} unit
 * @param {Alter=} opt_alter, default is Alter.MOVE.
 * @return {!OldRange}
 */
OldRange.prototype.endOf = function(unit, opt_alter) {};

/**
 * @param {!Editor.RegExp} regexp
 * @return {?Array.<string>}
 */
OldRange.prototype.match = function(regexp) {};

/**
 * @param {Unit} unit
 * @param {number=} opt_count, default is one.
 * @return {!OldRange}
 */
OldRange.prototype.move = function(unit, opt_count) {};

/**
 * @param {Unit} unit
 * @param {number=} opt_count, default is one.
 * @return {!OldRange}
 */
OldRange.prototype.moveEnd = function(unit, opt_count) {};

/**
 * @param {string} char_set
 * @param {number=} opt_count
 * @return {!OldRange}
 */
OldRange.prototype.moveEndWhile = function(char_set, opt_count) {};

/**
 * @param {Unit} unit
 * @param {number=} opt_count, default is one.
 * @return {!OldRange}
 */

OldRange.prototype.moveStart = function(unit, opt_count) {};

/**
 * @param {string} char_set
 * @param {number=} opt_count
 * @return {!OldRange}
 */
OldRange.prototype.moveStartWhile = function(char_set, opt_count) {};

/**
 * @param {!string} text
 */
OldRange.prototype.insertBefore = function(text) {};

/**
 * @param {!Editor.RegExp} regexp
 * @param {string|!function(string, ...):string} replacement
 * @return {?Array.<string>}
 */
OldRange.prototype.replace = function(regexp, replacement) {};

/**
 * @param {number} spelling
 */
OldRange.prototype.setSpelling = function(spelling) {};

/**
 * @param {!Object} style_dict
 */
OldRange.prototype.setStyle = function(style_dict) {};

/**
 * @param {string} syntax
 */
OldRange.prototype.setSyntax = function(syntax) {};

/**
 * @param {Unit} unit
 * @param {Alter=} opt_alter, default is Alter.MOVE.
 * @return {!OldRange}
 */
OldRange.prototype.startOf = function(unit, opt_alter) {};

/** @return {!OldRange} */
OldRange.prototype.toLocaleLowerCase = function() {}

/** @return {!OldRange} */
OldRange.prototype.toLocaleUpperCase = function() {}

/** @return {!OldRange} */
OldRange.prototype.toLowerCase = function() {}

/** @return {!OldRange} */
OldRange.prototype.toUpperCase = function() {}
