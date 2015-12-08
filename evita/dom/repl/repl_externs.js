// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @type {!Object} */
var repl;

/**
 * @param {*} object
 * @return {undefined}
 */
repl.describe = function(object) {};

/**
 * @param {string} scriptPath
 * @param {!Object=} opt_options
 * @return {!Promise}
 *
 * Options:
 *  encoding: string
 *  verbose: boolean
 */
repl.load = function(scriptPath, opt_options) {};

/**
  * @param{*} value
  * @param{number=} opt_maxLevel Default is 10.
  * @param{number=} opt_maxLength Default is 10.
  * @return {string}
  */
repl.stringify = function(value, opt_maxLevel, opt_maxLength) {};
