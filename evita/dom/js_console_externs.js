// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @struct
 * @return {undefined}
 */
function JsConsole() {};

/**
 * @param {*} reason
 */
JsConsole.errorHandler = function(reason) {};

/** @type {function(Window)} */
JsConsole.prototype.activate;

/** @type {function()} */
JsConsole.prototype.backwardHistory;

/** @type {!Document} */
JsConsole.prototype.document;

/** @param {string} text */
JsConsole.prototype.emit = function(text) {};

/** @type {function()} */
JsConsole.prototype.emitPrompt;

/**
 * @param {*} reason
 */
JsConsole.prototype.errorHandler = function(reason) {};

/** @type {function()} */
JsConsole.prototype.evalLastLine;

/** @type {function()} */
JsConsole.prototype.forwardHistory;

/** @type {function()} */
JsConsole.prototype.useHistory;
