// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @expose @type{!Object} */
var Os;

/**
 * @expose
 * @constructor
 */
Os.File = function() {};

/** @expose
 *  @constructor
 *  @param {number} winLastError
 */
Os.File.Error = function(winLastError) {};

/**
 * @expose
 * @typedef {{
 *    isDir: boolean,
 *    isSymLink: boolean,
 *    lastModificationDate: !Date,
 *    readonly: boolean,
 *    size: number
 * }}
 */
Os.File.Info;

/**
 * @expose
 * @param {string} dir_name
 * @param {string} prefix
 * @return {string}
 */
Os.File.makeTempFileName = function(dir_name, prefix) {};

/**
 * @expose
 * @param {string} file_name
 * @param {string=} opt_mode
 * @return {!Promise.<!Os.File>}
 */
Os.File.open = function(file_name, opt_mode) {};

/**
 * @expose
 * @param {string} file_name
 * @return {!Promise.<!Os.File.Info>}
 */
Os.File.stat = function(file_name) {};

/**
 * @expose
 * @return {!Promise.<number|!Os.File.Error>}
 */
Os.File.prototype.close = function() {};

/**
 * @expose
 * @param {!ArrayBufferView} buffer
 * @return {!Promise.<number|!Os.File.Error>}
 */
Os.File.prototype.read = function(buffer) {};

/**
 * @expose
 * @param {!ArrayBufferView} buffer
 * @return {!Promise.<number|!Os.File.Error>}
 */
Os.File.prototype.write = function(buffer) {};
