// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!Document} document
 */
repl.JsConsole = function(document) {};

/** @type {function()} */
repl.JsConsole.prototype.backwardHistory;

/** @type {!Document} */
repl.JsConsole.prototype.document;

/** @param {string} text */
repl.JsConsole.prototype.emit = function(text) {};

/** @type {function()} */
repl.JsConsole.prototype.emitPrompt;

/** @type {function()} */
repl.JsConsole.prototype.evalLastLine;

/** @type {function()} */
repl.JsConsole.prototype.forwardHistory;

/** @type {function()} */
repl.JsConsole.prototype.freshLine;

/** @type {!Range} */
repl.JsConsole.prototype.range;

/** @type {function()} */
repl.JsConsole.prototype.useHistory;
