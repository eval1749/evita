// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!Document} document
 */
repl.JsConsole = function(document) {};

/**
 * @param {string} keyCombination
 * @param {!Object} command
 */
repl.JsConsole.bindKey = function(keyCombination, command) {};

/** @type {!repl.JsConsole} */
repl.JsConsole.instance;

/** @type {function()} */
repl.JsConsole.prototype.backwardHistory;

/** @type {!Document} */
repl.JsConsole.prototype.document;

/** @type {function()} */
repl.JsConsole.prototype.emitPrompt;

/** @type {function()} */
repl.JsConsole.prototype.evalLastLine;

/** @type {function()} */
repl.JsConsole.prototype.forwardHistory;

/** @type {string} */
repl.JsConsole.prototype.lastLine;

/** @type {number} */
repl.JsConsole.prototype.lastLineStart;

/** @type {function()} */
repl.JsConsole.prototype.useHistory;
