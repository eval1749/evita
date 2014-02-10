// Copyright (C) 2014 by Project Vogue.
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
 *  - Map
 *  - Promise
 *  - Set
 *  - String
 */

/**
 * @template T
 * @param {function(T, number?, Array?): *} callback.
 * @param {Object=} this_arg
 * @return {T|null}
 */
Array.prototype.find = function(callback, this_arg) {};

/** @typedef {Array} */
var Iterable;

//////////////////////////////////////////////////////////////////////
//
// Map
//
/**
 * @constructor
 * @param {Iterable=} iterable
 */
var Map = function(iterable) {};

Map.prototype.clear = function() {};

/**
 * @param {*} key
 */
Map.prototype.delete = function(key) {};

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
 * @return {Array.<T>}
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
 * @return {Array}
 */
Map.prototype.values = function() {};

//////////////////////////////////////////////////////////////////////
//
// Promise
//

/**
 * @constructor
 * @template T
 * @param {function(T=)} onFulfilled.
 * @param {function(*=)} onRejected.
 */
var Promise = function(onFulfilled, onRejected) {};

/**
 * @param {!Iterable.<Promise>} iterable.
 * @return {!Promise}
 */
Promise.all = function(iterable){};

/**
 * @param {*} value.
 * @return {!Promise}
 */
Promise.cast = function(value) {};

/**
 * @param {*} value.
 * @return {{promise: Promise, resolve: function(*), reject: function(*)}}
 */
Promise.defer = function() {};

/**
 * @param {!Iterable.<Promise>} iterable.
 * @return {!Promise}
 */
Promise.race = function(iterable){};

/**
 * @param {*} value
 * @return {!Promise}
 */
Promise.rekect = function(value){};

/**
 * @param {*} value
 * @return {!Promise}
 */
Promise.resolve = function(value){};

/**
 * @template T
 * @param {function(T=)} onRejected
 * @return {!Promise}
 */
Promise.prototype.catch= function(onRejected) {};

/**
 * @template T
 * @param {function(T)} onFulfilled.
 * @param {function()=} onRejected
 * @return {!Promise.<T>}
 */
Promise.prototype.chain = function(onRejected) {};

/**
 * @template T
 * @param {function(T)} onFulfilled.
 * @param {function()=} opt_onRejected
 * @return {!Promise.<T>}
 */
Promise.prototype.then = function(onFulfilled, opt_onRejected) {};

/**
 * @constructor
 * @param {Iterable=} opt_iterable
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
 * @param {T} key
 * @return {boolean}
 */
Set.prototype.has = function(key) {};

/**
 * @template T
 * @return {Array.<T>}
 */
Set.prototype.keys = function() {};

/** @type {number} */
Set.prototype.size;

/**
 * @param {string} searchString
 * @param {number=} opt_position
 * @return {boolean}
 */
String.prototype.endsWith = function(searchString, opt_position) {};

/**
 * @param {number} count
 * @return {string}
 */
String.prototype.repeat = function(count) {};

/**
 * @param {string} searchString
 * @param {number=} opt_position
 * @return {boolean}
 */
String.prototype.startsWith = function(searchString, opt_position) {};
