// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!ScriptTextProvider} provider
 */
var ScriptModuleLoader = function(provider) {};

/**
 * @param {string} specifier
 * @return {!Promise<!Array<string>>} A list of module full name.
 */
ScriptModuleLoader.prototype.load = function(specifier) {};

/**
 * @param {string} specifier
 */
ScriptModuleLoader.prototype.unload = function(specifier) {};
