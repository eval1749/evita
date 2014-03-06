// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @expose
 * @constructor
 */
Os.FileHandle = function() {};

/**
 * @expose
 * @param {!ArrayBufferView} buffer
 * @return {!Promise.<number|!Os.File.Error>}
 */
Os.FileHandle.prototype.read = function(buffer) {};

/**
 * @expose
 * @param {!ArrayBufferView} buffer
 * @param {!function(number, number)} callback
 */
Os.FileHandle.prototype.read_ = function(buffer, callback) {};

/**
 * @expose
 * @param {!ArrayBufferView} buffer
 * @return {!Promise.<number|!Os.File.Error>}
 */
Os.FileHandle.prototype.write = function(buffer) {};

/**
 * @expose
 * @param {!ArrayBufferView} buffer
 * @param {!function(number, number)} callback
 */
Os.FileHandle.prototype.write_ = function(buffer, callback) {};
