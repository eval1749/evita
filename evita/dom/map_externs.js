// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

'use strict';

/**
 * @fileoverview Map
 * @externs
 */

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
