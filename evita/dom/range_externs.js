// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview Range
 * See types_externs.js for Range constructor.
 * @externs
 */

/** @expose @type {!Document} */
Range.prototype.document;

/** @expose @type {Position} */
Range.prototype.end = 0;

/** @expose @type {Position} */
Range.prototype.start = 0;


/** @expose @type {!string} */
Range.prototype.text = '';

/** @return {!Range} */
Range.prototype.capitalize = function() {}

/**
 * @param {Position} position
 */
Range.prototype.collapseTo = function(position) {};

/** @type {function()} */
Range.prototype.copy = function() {};

/**
 * @param {Unit} unit
 * @param {number=} opt_count, defualt is one.
 * @return {!Range}
 */
Range.prototype.delete = function(unit, opt_count) {};

/**
 * @param {Unit} unit
 * @param {Alter=} opt_alter, default is Alter.MOVE.
 */
Range.prototype.endOf = function(unit, opt_alter) {};

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
 * @param {Unit} unit
 * @param {number=} opt_count, defualt is one.
 * @return {!Range}
 */
Range.prototype.moveStart = function(unit, opt_count) {};

/**
 * @param {!string} text
 */
Range.prototype.insertBefore = function(text) {};

/** @type {function()} */
Range.prototype.paste = function() {};

/**
 * @param {Unit} unit
 * @param {Alter=} opt_alter, default is Alter.MOVE.
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
