// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * @externs
 */

var commands;

/**
 * @param {string} name
 * @param {function(!TextWindow)} procedure
 * @param {string} description
 */
commands.Command = function(name, procedure, description) {};

/**
 * @param {string} name
 * @return {?Command}
 */
command.Command.query = function(name) {};

/**
 * @param {!Command} command
 */
command.Command.register = function(command) {};

/** @type {string} */
commands.Command.prototype.description;

/** @type {string} */
commands.Command.prototype.name;

/**
 * @param {!TextWindow} window
 * @param {number=} opt_count
 */
commands.Command.prototype.execute = function(window, opt_count) {};
