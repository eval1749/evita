// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview ECMAScript 6, aka Harmony classes
 * @externs
 *  http://people.mozilla.org/~jorendorff/es6-draft.html
 *  https://github.com/domenic/promises-unwrapping/blob/master/README.md
 *
 * This file contains following ES6 classes:
 *  - Map
 *  - Promise
 */
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
 * @return {Array}
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
 * @return {Promise}
 */
Promise.all = function(iterable){};

/**
 * @param {*} value.
 * @return {Promise}
 */
Promise.cast = function(value) {};

/**
 * @param {*} value.
 * @return {{promise: Promise, resolve: function(*), reject: function(*)}}
 */
Promise.defer = function() {};

/**
 * @param {!Iterable.<Promise>} iterable.
 * @return {Promise}
 */
Promise.race = function(iterable){};

/**
 * @param {*} value
 * @return {Promise}
 */
Promise.rekect = function(value){};

/**
 * @param {*} value
 * @return {Promise}
 */
Promise.resolve = function(value){};

/**
 * @template T
 * @param {function(T=)} onRejected
 * @return {Promise}
 */
Promise.prototype.catch= function(onRejected) {};

/**
 * @template T
 * @param {function(T)} onFulfilled.
 * @param {function()=} onRejected
 * @return {Promise.<T>}
 */
Promise.prototype.chain = function(onRejected) {};

/**
 * @template T
 * @param {function(T)} onFulfilled.
 * @param {function()=} opt_onRejected
 * @return {Promise.<T>}
 */
Promise.prototype.then = function(onFulfilled, opt_onRejected) {};
