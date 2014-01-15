// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview Polyfill
 * @externs
 */

var Array;
var Function;
var Math;
var Object;
var String;

/**
 * @template T
 * @param {function(T, number?, Array?): *} callback.
 * @param {Object=} this_arg
 * @return {T|null}
 */
Array.prototype.find = function(callback, this_arg) {};

/**
 * @param {function(T, number?, Array?): *} callback.
 * @param {Object=} this_arg
 * @return {Array}
 */
Array.prototype.forEach = function(callback, this_arg) {};

/**
 * @param {!string} separator
 * @return {!string}
 */
Array.prototype.join = function(separator) {};

/**
 * @param {function(T, number?, Array?): *} callback.
 * @param {Object=} this_arg
 * @return {Array}
 */
Array.prototype.map = function(callback, this_arg) {};

/**
 * @param {...} var_args
 */
Array.prototype.push = function(var_args) {};

/**
 * @template T
 * @param {number} start
 * @param {number=} opt_end
 * @return {Array.<T>}
 */
Array.prototype.slice = function(start, opt_end) {};

/**
 * @template T
 * @param {function(T): boolean} predicate.
 * @param {*=} this_arg
 * @return {boolean}
 */
Array.prototype.some = function(predicate, this_arg) {};

/**
 * @param {*} this_arg
 * @param {...} var_args
 * @return {*}
 */
Function.prototype.call = function(this_arg, var_args) {};

/**
 * @param {...number} var_args
 * @return {number}
 */
Math.max = function(var_args) {};

/**
 * @param {...number} var_args
 * @return {number}
 */
Math.min = function(var_args) {};

/**
 * @template T
 * @param {T} proto.
 * @param {*} opt_properties.
 * @return {T}
 */
Object.prototype.create = function(proto, opt_properties) {};

/** @expose @type {number} */
String.prototype.length;

/**
 * @param {number} index
 * @return {string}
 */
String.prototype.charAt = function(index) {};

/**
 * @param {number} index
 * @return {number}
 */
String.prototype.charCodeAt = function(index) {};

/**
 * @param {number} start
 * @param {number=} length
 * @return {string}
 */
String.prototype.substr = function(start, length) {};

/**
 * @param {number} start
 * @param {number=} end
 * @return {string}
 */
String.prototype.substring = function(start, end) {};

/** @return {string} */
String.prototype.toLocaleLowerCase = function() {};

/** @return {string} */
String.prototype.toLocaleUpperCase = function() {};

/** @typedef {Array} */
var Iterable;
