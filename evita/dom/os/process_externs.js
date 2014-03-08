// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @expose
 * @constructor
 */
Os.Process = function() {};

/**
 * @expose
 * @param {string} command_name
 * @return {!Promise.<!Os.Process>}
 */
Os.Process.open = function(command_name) {};

/**
 * @expose
 * @return {!Promise.<number>}
 */
Os.Process.prototype.close = function() {};

/**
 * @expose
 * @param {!ArrayBufferView} buffer
 * @return {!Promise.<number>}
 */
Os.Process.prototype.read = function(buffer) {};

/**
 * @expose
 * @param {!ArrayBufferView} buffer
 * @return {!Promise.<number>}
 */
Os.Process.prototype.write = function(buffer) {};
