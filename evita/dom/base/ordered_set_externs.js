// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @template T
 * @constructor
 * @param {T} data
 */
base.OrderedSetNode = function(data) {};

/**
 * @type {T}
 */
base.OrderedSetNode.prototype.data;

/**
 * @template T
 * @return {?base.OrderedSetNode.<T>}
 */
base.OrderedSetNode.prototype.next = function() {};

/**
 * @template T
 * @return {?base.OrderedSetNode.<T>}
 */
base.OrderedSetNode.prototype.previous = function() {};

/**
 * @template T
 * @constructor
 * @param {!function(T, T):boolean} less
 */
base.OrderedSet = function(less) {};

/**
 * @type {T}
 */
base.OrderedSet.prototype.maximum;

/**
 * @type {T}
 */
base.OrderedSet.prototype.minimum;

/**
 * @type {number}
 */
base.OrderedSet.prototype.size;

/**
 * @param {T} data
 */
base.OrderedSet.prototype.add = function(data) {};

/**
 * @type {!function()}
 */
base.OrderedSet.prototype.clear = function() {};

/**
 * @template T
 * @param {T} data
 * @return {?base.OrderedSetNode.<T>}
 */
base.OrderedSet.prototype.find = function(data) {};

/**
 * @template T
 * @param {!function(T)} callback
 */
base.OrderedSet.prototype.forEach = function(callback) {};

/**
 * @template T
 * @param {T} data
 * @return {?base.OrderedSetNode.<T>}
 */
base.OrderedSet.prototype.lowerBound= function(data) {};

/**
 * @template T
 * @param {T} data
 */
base.OrderedSet.prototype.remove = function(data) {};

/**
 * @typedef {base.OrderedSetNode}
 */
var OrderedSetNode;

/**
 * @typedef {base.OrderedSet}
 */
var OrderedSet;

// TODO(eval1749): Once closure compiler recognize type annotation in
// constructor, we don't need to have following annotations.
/** @type {T} */
base.OrderedSetNode.prototype.data_;

/** @type {number} */
base.OrderedSetNode.prototype.priority_;

/** @type {OrderedSetNode<T>} */
base.OrderedSet.prototype.root_;

/** @type {number} */
base.OrderedSet.prototype.size_;

