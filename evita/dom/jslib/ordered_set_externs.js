// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @template T
 * @constructor
 * @param {T} data
 */
function OrderedSetNode(data) {}

/**
 * @template T
 * @type {T}
 */
OrderedSetNode.prototype.data;

/**
 * @template T
 * @return {?OrderedSetNode.<T>}
 */
OrderedSetNode.prototype.next = function() {};

/**
 * @template T
 * @return {?OrderedSetNode.<T>}
 */
OrderedSetNode.prototype.previous = function() {};

/**
 * @template T
 * @constructor
 * @param {!function(T, T):boolean} less
 */
function OrderedSet(less) {}

/**
 * @type {number}
 */
OrderedSet.prototype.size;

/**
 * @template T
 * @param {T} data
 */
OrderedSet.prototype.add = function(data) {};

/**
 * @template T
 * @param {T} data
 * @return {?OrderedSetNode.<T>}
 */
OrderedSet.prototype.find = function(data) {};

/**
 * @template T
 * @param {!function(T)} callback
 */
OrderedSet.prototype.forEach = function(callback) {};

/**
 * @template T
 * @param {T} data
 * @return {?OrderedSetNode.<T>}
 */
OrderedSet.prototype.lowerBound= function(data) {};

/**
 * @template T
 * @param {!OrderedSetNode.<T>} node
 */
OrderedSet.prototype.remove = function(node) {};
