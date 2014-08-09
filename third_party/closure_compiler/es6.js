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
 *  - Generator
 *  - Iterable
 *  - Iterator
 *  - Map
 *  - Promise
 *  - Set
 *  - String
 *  - Symbol
 *  - WeakMap
 *  - WeakSet
 */

/**
 * @typedef {Function}
 */
var Iterator;

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

/**
 * @typedef {Array|Map|Set}
 */
var Iterable;

//////////////////////////////////////////////////////////////////////
//
// Generator
//
/**
 * @constructor
 */
function Generator() {}

/**
 * @template T
 * @return {{done: boolean, value: T}}
 */
Generator.prototype.next = function() {};

//////////////////////////////////////////////////////////////////////
//
// ArrayBuffer
//
/**
 * @constructor
 * @param {number} byteLength
 */
var ArrayBuffer = function(byteLength) {};

/**
 * @param {*} value
 * @return boolean
 */
ArrayBuffer.prototype.isValue = function(value){};


//////////////////////////////////////////////////////////////////////
//
// ArrayBufferView
//
/**
 * @constructor
 */
function ArrayBufferView() {}

/** @type {!ArrayBuffer} */
ArrayBufferView.prototype.buffer;

/** @type {number} */
ArrayBufferView.prototype.byteLength;

/** @type {number} */
ArrayBufferView.prototype.byteOffset;

//////////////////////////////////////////////////////////////////////
//
// TypedArray
//
/**
 * @constructor
 * @extends {ArrayBufferView}
 */
function TypedArray() {}

/** @type {number} */
TypedArray.BYTES_PER_ELEMENT;

/** @type {number} */
TypedArray.prototype.length;

/**
 * @param {!TypedArray} array
 * @param {number=} opt_offset
 */
TypedArray.prototype.set = function(array, opt_offset) {};

//////////////////////////////////////////////////////////////////////
//
// Float32Array
//
/** @const @type {number} */
Float32Array.BYTES_PER_ELEMENT = 4;

/**
 * @constructor
 * @extends {TypedArray}
 * @param {number|!TypedArray|!ArrayBuffer} thing
 * @param {number=} opt_offset
 * @param {number=} opt_length
 */
function Float32Array(thing, opt_offset, opt_length) {}

/**
 * @param {number} begin
 * @param {number=} opt_end
 * @return {!Float32Array}
 */
Float32Array.prototype.subarray = function(begin, opt_end) {};

//////////////////////////////////////////////////////////////////////
//
// Float64Array
//
/** @const @type {number} */
Float64Array.BYTES_PER_ELEMENT = 8;

/**
 * @constructor
 * @extends {TypedArray}
 * @param {number|!TypedArray|!ArrayBuffer} thing
 * @param {number=} opt_offset
 * @param {number=} opt_length
 */
function Float64Array(thing, opt_offset, opt_length) {}

/**
 * @param {number} begin
 * @param {number=} opt_end
 * @return {!Float64Array}
 */
Float64Array.prototype.subarray = function(begin, opt_end) {};

//////////////////////////////////////////////////////////////////////
//
// Int16Array
//
/** @const @type {number} */
Int16Array.BYTES_PER_ELEMENT = 2;

/**
 * @constructor
 * @extends {TypedArray}
 * @param {number|!TypedArray|!ArrayBuffer} thing
 * @param {number=} opt_offset
 * @param {number=} opt_length
 */
function Int16Array(thing, opt_offset, opt_length) {}

/**
 * @param {number} begin
 * @param {number=} opt_end
 * @return {!Int16Array}
 */
Int16Array.prototype.subarray = function(begin, opt_end) {};

//////////////////////////////////////////////////////////////////////
//
// Int32Array
//
/** @const @type {number} */
Int32Array.BYTES_PER_ELEMENT = 4;

/**
 * @constructor
 * @extends {TypedArray}
 * @param {number|!TypedArray|!ArrayBuffer} thing
 * @param {number=} opt_offset
 * @param {number=} opt_length
 */
function Int32Array(thing, opt_offset, opt_length) {}

/**
 * @param {number} begin
 * @param {number=} opt_end
 * @return {!Int32Array}
 */
Int32Array.prototype.subarray = function(begin, opt_end) {};

//////////////////////////////////////////////////////////////////////
//
// Int8Array
//
/** @const @type {number} */
Int8Array.BYTES_PER_ELEMENT = 2;

/**
 * @constructor
 * @extends {TypedArray}
 * @param {number|!TypedArray|!ArrayBuffer} thing
 * @param {number=} opt_offset
 * @param {number=} opt_length
 */
function Int8Array(thing, opt_offset, opt_length) {}

/**
 * @param {number} begin
 * @param {number=} opt_end
 * @return {!Int8Array}
 */
Int8Array.prototype.subarray = function(begin, opt_end) {};

//////////////////////////////////////////////////////////////////////
//
// Uint16Array
//
/** @const @type {number} */
Uint16Array.BYTES_PER_ELEMENT = 2;

/**
 * @constructor
 * @extends {TypedArray}
 * @param {number|!TypedArray|!ArrayBuffer} thing
 * @param {number=} opt_offset
 * @param {number=} opt_length
 */
function Uint16Array(thing, opt_offset, opt_length) {}

/**
 * @param {number} begin
 * @param {number=} opt_end
 * @return {!Uint16Array}
 */
Uint16Array.prototype.subarray = function(begin, opt_end) {};

//////////////////////////////////////////////////////////////////////
//
// Uint32Array
//
/** @const @type {number} */
Uint32Array.BYTES_PER_ELEMENT = 4;

/**
 * @constructor
 * @extends {TypedArray}
 * @param {number|!TypedArray|!ArrayBuffer} thing
 * @param {number=} opt_offset
 * @param {number=} opt_length
 */
function Uint32Array(thing, opt_offset, opt_length) {}

/**
 * @param {number} begin
 * @param {number=} opt_end
 * @return {!Uint32Array}
 */
Uint32Array.prototype.subarray = function(begin, opt_end) {};

//////////////////////////////////////////////////////////////////////
//
// Uint8Array
//
/** @const @type {number} */
Uint8Array.BYTES_PER_ELEMENT = 1;

/**
 * @constructor
 * @extends {TypedArray}
 * @param {number|!TypedArray|!ArrayBuffer} thing
 * @param {number=} opt_offset
 * @param {number=} opt_length
 */
function Uint8Array(thing, opt_offset, opt_length) {}

/**
 * @param {number} begin
 * @param {number=} opt_end
 * @return {!Uint8Array}
 */
Uint8Array.prototype.subarray = function(begin, opt_end) {};

//////////////////////////////////////////////////////////////////////
//
// Uint8ClampedArray
//
/** @const @type {number} */
Uint8ClampedArray.BYTES_PER_ELEMENT = 1;

/**
 * @constructor
 * @extends {TypedArray}
 * @param {number|!TypedArray|!ArrayBuffer} thing
 * @param {number=} opt_offset
 * @param {number=} opt_length
 */
function Uint8ClampedArray(thing, opt_offset, opt_length) {}

/**
 * @param {number} begin
 * @param {number=} opt_end
 * @return {!Uint8ClampedArray}
 */
Uint8ClampedArray.prototype.subarray = function(begin, opt_end) {};

//////////////////////////////////////////////////////////////////////
//
// Map
//
/**
 * @constructor
 * @template K, V
 * @param {Iterable=} iterable
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
 * @return {!Iterator}
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
 * @return {!Iterator}
 */
Map.prototype.values = function() {};

//////////////////////////////////////////////////////////////////////
//
// Promise
//

/**
 * @constructor
 * @template T
 * @param {!function(T=)} onFulfilled
 * @param {!function(*=)} onRejected
 */
var Promise = function(onFulfilled, onRejected) {};

/**
 * @param {!Iterable.<!Promise>} iterable
 * @return {!Promise}
 */
Promise.all = function(iterable) {};

/**
 * @param {*} value
 * @return {!Promise}
 */
Promise.accept = function(value) {};

/**
 * @param {*} value
 * @return {{promise: !Promise, resolve: !function(*), reject: !function(*)}}
 */
Promise.defer = function(value) {};

/**
 * @param {!Iterable.<!Promise>} iterable
 * @return {!Promise}
 */
Promise.race = function(iterable) {};

/**
 * @param {*} value
 * @return {!Promise}
 */
Promise.reject = function(value) {};

/**
 * @template T
 * @param {T} value
 * @return {!Promise.<T>}
 */
Promise.resolve = function(value) {};

/**
 * @param {!function(*)} onRejected
 * @return {!Promise}
 */
Promise.prototype.catch = function(onRejected) {};

/**
 * @template T, S
 * @param {!function(T):S} onFulfilled
 * @param {!function(*)=} opt_onRejected
 * @return {!Promise.<S>}
 */
Promise.prototype.then = function(onFulfilled, opt_onRejected) {};

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
 * @return {!Iterator}
 */
Set.prototype.keys = function() {};

/** @type {number} */
Set.prototype.size;

/**
 * @template T
 * @return {!Iterator}
 */
Set.prototype.values = function() {};

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

/**
 * @constructor
 * @param {string=} opt_name
 * @return {!Symbol}
 */
function Symbol(opt_name) {}

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
