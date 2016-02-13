// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!TextSelection} selection
 * @param {!repl.JsConsole} commandLoop
 */
repl.JsCompleter = function(selection, commandLoop) {};

/**
 * @param {!repl.JsConsole} commandLoop
 * @param {!TextSelection} selection
 * @return {!repl.JsCompleter}
 */
repl.JsCompleter.ensure = function(commandLoop, selection) {};


/** @return {boolean} */
repl.JsCompleter.prototype.perform = function() {};
