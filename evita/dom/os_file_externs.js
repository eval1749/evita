// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @expose @type{!Object} */
var Os;

/** @expose @type{!Object} */
Os.File;

/** @expose @constructor */
Os.File.Error

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
 * @param {string} file_name
 * @param {string=} opt_mode
 * @return {!Promise.<!Os.FileHandle>}
 */
Os.File.open = function(file_name, opt_mode) {};

/**
 * @expose
 * @param {string} file_name
 * @param {string} mode
 * @param {!function(*)} callback
 */
Os.File.open_ = function(file_name, mode, callback) {};

/**
 * @expose
 * @param {string} file_name
 * @return {!Promise.<!Os.File.Info>}
 */
Os.File.stat = function(file_name) {};

/**
 * @expose
 * @param {string} file_name
 * @param {!function(!Os.File.Info)} callback
 */
Os.File.stat_ = function(file_name, callback) {};
