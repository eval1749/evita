// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Range
 * See types_externs.js for Range constructor.
 * @externs
 */

/** @enum {string} */
Range.Case = {
  CAPITALIZED_TEXT: 'CAPITALIZED_TEXT',
  CAPITALIZED_WORDS: 'CAPITALIZED_WORDS',
  LOWER: 'LOWER',
  MIXED: 'MIXED',
  UPPER: 'UPPER'
};

/** @expose @type {!Document} */
Range.prototype.document;

/** @expose @type {!TextOffset} */
Range.prototype.end = 0;

/** @expose @type {!TextOffset} */
Range.prototype.start = 0;


/** @expose @type {!string} */
Range.prototype.text = '';

/** @return {!Range} */
Range.prototype.capitalize = function() {}

/** @type {boolean} */
Range.prototype.collapsed;

/**
 * @return {!Range.Case}
 */
Range.prototype.analyzeCase = function() {};

/**
 * @param {!TextOffset} position
 */
Range.prototype.collapseTo = function(position) {};

/**
 * @param {Unit} unit
 * @param {number=} opt_count, defualt is one.
 * @return {!Range}
 */
Range.prototype.delete = function(unit, opt_count) {};

/**
 * @param {Unit} unit
 * @param {Alter=} opt_alter, default is Alter.MOVE.
 * @return {!Range}
 */
Range.prototype.endOf = function(unit, opt_alter) {};


/**
 * @param {!Editor.RegExp} regexp
 * @return {?Array.<string>}
 */
Range.prototype.match = function(regexp) {};

/**
 * @param {Unit} unit
 * @param {number=} opt_count, defualt is one.
 * @return {!Range}
 */
Range.prototype.move = function(unit, opt_count) {};

/**
 * @param {Unit} unit
 * @param {number=} opt_count, defualt is one.
 * @return {!Range}
 */
Range.prototype.moveEnd = function(unit, opt_count) {};

/**
 * @param {string} char_set
 * @param {number=} opt_count
 * @return {!Range}
 */
Range.prototype.moveEndWhile = function(char_set, opt_count) {};

/**
 * @param {Unit} unit
 * @param {number=} opt_count, defualt is one.
 * @return {!Range}
 */
Range.prototype.moveStart = function(unit, opt_count) {};

/**
 * @param {string} char_set
 * @param {number=} opt_count
 * @return {!Range}
 */
Range.prototype.moveStartWhile = function(char_set, opt_count) {};

/**
 * @param {!string} text
 */
Range.prototype.insertBefore = function(text) {};

/**
 * @param {!Editor.RegExp} regexp
 * @param {string|!function(string, ...):string} replacement
 * @return {?Array.<string>}
 */
Range.prototype.replace = function(regexp, replacement) {};

/**
 * @param {number} spelling
 */
Range.prototype.setSpelling = function(spelling) {};

/**
 * @param {!Object} style_dict
 */
Range.prototype.setStyle = function(style_dict) {};

/**
 * @param {Unit} unit
 * @param {Alter=} opt_alter, default is Alter.MOVE.
 * @return {!Range}
 */
Range.prototype.startOf = function(unit, opt_alter) {};

/** @return {!Range} */
Range.prototype.toLocaleLowerCase = function() {}

/** @return {!Range} */
Range.prototype.toLocaleUpperCase = function() {}

/** @return {!Range} */
Range.prototype.toLowerCase = function() {}

/** @return {!Range} */
Range.prototype.toUpperCase = function() {}
