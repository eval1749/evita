// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @type {!Object} */
var repl;

/**
 * @constructor
 */
repl.Console = function() {};

/**
 * @param {*} expression
 * @param {...*} params
 */
repl.Console.prototype.assert = function(expression, params) {};

/** @type {!function()} */
repl.Console.prototype.clear;

/** @type {!TextDocument} */
repl.Console.prototype.document;

/** @param {string} string */
repl.Console.prototype.emit = function(string) {};

/** @type {!function()} */
repl.Console.prototype.freshLine;

/**
 * @param {...*} params
 */
repl.Console.prototype.log = function(params) {};
