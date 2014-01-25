// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @const @type {function(*, number=, number=) : string} */
JsConsole.stringify;

/** @type {function(Window)} */
JsConsole.prototype.activate;

/** @type {function()} */
JsConsole.prototype.backwardHistory;

/** @type {!Document} */
JsConsole.prototype.document;

/** @type {function(string)} */
JsConsole.prototype.emit;

/** @type {function()} */
JsConsole.prototype.emitPrompt;

/** @type {function()} */
JsConsole.prototype.evalLastLine;

/** @type {function()} */
JsConsole.prototype.forwardHistory;

/** @type {Array.<string>} */
JsConsole.prototype.history;

/** @type {number} */
JsConsole.prototype.history_index;

/** @type {number} */
JsConsole.prototype.line_number;

/** @type {function()} */
JsConsole.prototype.newWindow;

/** @type {!Range} */
JsConsole.prototype.range;

/** @type {function()} */
JsConsole.prototype.useHistory;
