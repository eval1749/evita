// Copyright (C) 2014 by Project Vogue
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview ECMAScript 6, aka Harmony classes
 * @externs
 *  http://people.mozilla.org/~jorendorff/es6-draft.html
 *  https://github.com/domenic/promises-unwrapping/blob/master/README.md
 *
 * This file contains following ES6 classes:
 *  - Array
 *  - Iterable
 *  - Iterator
 *  - Map
 *  - Set
 *  - String
 *  - WeakMap
 *  - WeakSet
 */

/**
 * @template T
 * @param {T} thing
 * @param {function(?):T=} opt_mapper
 * @param {?=} opt_this
 * @return {!Array.<T>}
 */
Array.from = function(thing, opt_mapper, opt_this) {};

/**
 * @template T
 * @this {Array.<T>}
 * @param {T} value
 * @param {number=} opt_start
 * @param {number=} opt_end
 */
Array.prototype.fill = function(value, opt_start, opt_end) {};

/**
 * @template T
 * @param {function(T, number=, Array=): *} callback
 * @param {Object=} opt_thisArg
 * @return {T|null}
 */
Array.prototype.find = function(callback, opt_thisArg) {};

/**
 * @template T
 * @param {function(T, number=, Array=): *} callback
 * @param {Object=} opt_thisArg
 * @return {number}
 */
Array.prototype.findIndex = function(callback, opt_thisArg) {};

//////////////////////////////////////////////////////////////////////
//
// Map
//
/**
 * @constructor
 * @template K, V
 * @param {(Array|Iterable)=} iterable
 */
var Map = function(iterable) {};

Map.prototype.clear = function() {};

/**
 * @template T
 * @return {!Iterator}
 */
Map.prototype.entries = function() {};

/**
 * @param {*} key
 */
Map.prototype.delete = function(key) {};

/**
 * @template K, V
 * @param {!function(V, K, !Map.<K, V>)} callback
 */
Map.prototype.forEach = function(callback) {};

/**
 * @template T
 * @param {*} key
 * @return {T|null}
 */
Map.prototype.get = function(key) {};

/**
 * @param {*} key
 * @return {boolean}
 */
Map.prototype.has = function(key) {};

/**
 * @template T
 * @return {!Iterable.<T>}
 */
Map.prototype.keys = function() {};

/**
 * @param {*} key
 * @param {*} value
 */
Map.prototype.set = function(key, value) {};

/**
 * @type {number}
 */
Map.prototype.size;

/**
 * @template T
 * @return {!Iterable.<T>}
 */
Map.prototype.values = function() {};

/**
 * @constructor
 * @template T
 * @param {!Iterable=} opt_iterable
 */
var Set = function(opt_iterable) {};

/**
 * @template T
 * @param {T} key
 */
Set.prototype.add = function(key) {};

/** @type {function()} */
Set.prototype.clear = function() {};

/**
 * @template T
 * @param {T} key
 * @return {boolean}
 */
Set.prototype.delete = function(key) {};


/**
 * @template T
 * @return {!Iterator}
 */
Set.prototype.entries = function() {};

/**
 * @template T
 * @param {!function(T, T, !Set.<T>)} callback
 */
Set.prototype.forEach = function(callback) {};

/**
 * @template T
 * @param {T} key
 * @return {boolean}
 */
Set.prototype.has = function(key) {};

/**
 * @template T
 * @return {!Iterable.<T>}
 */
Set.prototype.keys = function() {};

/** @type {number} */
Set.prototype.size;

/**
 * @template T
 * @return {!Iterable.<T>}
 */
Set.prototype.values = function() {};

/**
 * @param {string} searchString
 * @param {number=} opt_position
 * @return {boolean}
 */
String.prototype.endsWith = function(searchString, opt_position) {};

/**
 * @param {string} searchString
 * @param {number=} opt_position
 * @return {boolean}
 */
String.prototype.startsWith = function(searchString, opt_position) {};

//////////////////////////////////////////////////////////////////////
//
// WeakMap
//
/**
 * @template KEY, VALUE
 * @constructor
 */
function WeakMap() {}

/**
 * @template KEY, VALUE
 * @this {WeakMap.<KEY, VALUE>}
 */
WeakMap.prototype.clear = function() {}

/**
 * @template KEY, VALUE
 * @this {WeakMap.<KEY, VALUE>}
 * @param {KEY} key
 */
WeakMap.prototype.delete = function(key) {}

/**
 * @template KEY, VALUE
 * @this {WeakMap.<KEY, VALUE>}
 * @param {KEY} key
 * @return {VALUE|undefined}
 */
WeakMap.prototype.get = function(key) {}

/**
 * @template KEY, VALUE
 * @this {WeakMap.<KEY, VALUE>}
 * @param {KEY} key
 * @return {boolean}
 */
WeakMap.prototype.has = function(key) {}

/**
 * @template KEY, VALUE
 * @this {WeakMap.<KEY, VALUE>}
 * @param {KEY} key
 * @param {VALUE} value
 */
WeakMap.prototype.set = function(key, value) {}

//////////////////////////////////////////////////////////////////////
//
// WeakSet
//
/**
 * @template KEY
 * @constructor
 */
function WeakSet() {}

/**
 * @template KEY
 * @this {WeakSet.<KEY>}
 * @param {KEY} key
 */
WeakSet.prototype.add = function(key) {}

/**
 * @template KEY
 * @this {WeakSet.<KEY>}
 */
WeakSet.prototype.clear = function() {}

/**
 * @template KEY
 * @this {WeakSet.<KEY>}
 * @param {KEY} key
 */
WeakSet.prototype.delete = function(key) {}

/**
 * @template KEY
 * @this {WeakSet.<KEY>}
 * @param {KEY} key
 * @return {KEY|undefined}
 */
WeakSet.prototype.get = function(key) {}

/**
 * @template KEY
 * @this {WeakSet.<KEY>}
 * @param {KEY} key
 * @return {boolean}
 */
WeakSet.prototype.has = function(key) {}
