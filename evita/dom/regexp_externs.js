// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @typedef {!Object}
 *  backward: boolean,
 *  global: boolean,
 *  ignoreCase: boolean,
 *  matchExact: boolean,
 *  matchWord: boolean,
 *  multiline: boolean,
 *  sticky: boolean,
 */
Editor.RegExpInit;

/**
 * @constructor
 * @param {string} source
 * @param {Editor.RegExpInit=} opt_init_dict
 */
Editor.RegExp = function(source, opt_init_dict) {};

/**
 * @struct {name: string, start: number, end: number}
 */
Editor.RegExp.Match;

/** @type {boolean} */
Editor.RegExp.prototype.backward;

/** @type {boolean} */
Editor.RegExp.prototype.global;

/** @type {boolean} */
Editor.RegExp.prototype.ignoreCase;

/** @type {boolean} */
Editor.RegExp.prototype.matchExact;

/** @type {boolean} */
Editor.RegExp.prototype.matchWord;

/** @type {boolean} */
Editor.RegExp.prototype.multiline;

/** @type {string} */
Editor.RegExp.prototype.source;

/** @type {boolean} */
Editor.RegExp.prototype.sticky;

/**
 * @param {string|!Document|!Range} target
 * @param {number=} opt_start
 * @param {number=} opt_end
 * @return {?Array.<string>}
 */
Editor.RegExp.prototype.exec = function(target, opt_start, opt_end) {};

/**
 * @param {!Document} document
 * @param {number} start
 * @param {number} end
 * @return {?Array.<{start: number, end: number}>}
 */
Editor.RegExp.prototype.execOnDocument_ = function(document, start, end) {};

/**
 * @param {string} string
 * @return {?Array.<{start: number, end: number}>}
 */
Editor.RegExp.prototype.execOnString_ = function(string) {};

/**
 * @param {string} replacement
 * @param {string|!Document|!Range} target
 * @param {number=} opt_start
 * @param {number=} opt_end
 * @return {?Array.<string>}
 */
Editor.RegExp.prototype.replace = function(replacement, target, opt_start,
                                           opt_end) {};
