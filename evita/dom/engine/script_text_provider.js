// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @interface */
const ScriptTextProvider = function() {};

/**
 * @param {string} specifier
 * @return {!Promise<string>}
 */
ScriptTextProvider.prototype.computeFullName = function(specifier) {};

/**
 * @param {string} fullName
 * @return {string}
 */
ScriptTextProvider.prototype.dirNameOf = function(fullName) {};

/**
 * @param {string} specifier
 * @param {string} dirName
 * @return {string}
 */
ScriptTextProvider.prototype.normalizeSpecifier = function(specifier, dirName) {
};

/**
 * @param {string} fullName
 * @param {string} referrer
 * @return {!Promise<string>}
 */
ScriptTextProvider.prototype.readScriptText = function(fullName, referrer) {};
