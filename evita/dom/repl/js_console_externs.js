// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @struct
 * @return {undefined}
 */
repl.JsConsole = function() {};

/**
 * @param {*} reason
 */
repl.JsConsole.errorHandler = function(reason) {};

/** @type {function(Window)} */
repl.JsConsole.prototype.activate;

/** @type {function()} */
repl.JsConsole.prototype.backwardHistory;

/** @type {!Document} */
repl.JsConsole.prototype.document;

/** @param {string} text */
repl.JsConsole.prototype.emit = function(text) {};

/** @type {function()} */
repl.JsConsole.prototype.emitPrompt;

/**
 * @param {*} reason
 */
repl.JsConsole.prototype.errorHandler = function(reason) {};

/** @type {function()} */
repl.JsConsole.prototype.evalLastLine;

/** @type {function()} */
repl.JsConsole.prototype.forwardHistory;

/** @type {function()} */
repl.JsConsole.prototype.useHistory;
