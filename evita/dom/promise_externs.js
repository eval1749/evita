// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview ECMAScript 6, aka Harmony classes
 * @externs
 *  http://people.mozilla.org/~jorendorff/es6-draft.html
 */

/**
 * @constructor
 * @template T
 * @param {function(T)} onFulfilled.
 * @param {function()} onRejected.
 */
var Promise = function(onFulfilled, onRejected) {};

/**
 * @param {!Iterable.<Promise>} iterable.
 * @return {Promise}
 */
Promise.all = function(iterable){};

/**
 * @param {!Iterable.<Promise>} iterable.
 * @return {Promise}
 */
Promise.race = function(iterable){};

/**
 * @param {*} value.
 * @return {Promise}
 */
Promise.cast = function(value) {};

/**
 * @template T
 * @param {function(T)} onFulfilled.
 * @param {function()=} opt_onRejected
 * @return {Promise.<T>}
 */
Promise.prototype.then = function(onFulfilled, opt_onRejected) {};

/**
 * @param {*} reason.
 * @return {Promise}
 */
Promise.prototype.reject = function(value) {};
