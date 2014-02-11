// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @expose @type{!Object} */
var Os;

/** @expose @type{!Object} */
Os.File;

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
 * @return {!Promise.<!Os.File.Info>}
 */
Os.File.stat = function(file_name) {};

/**
 * @expose
 * @param {string} file_name
 * @param {!function(!Os.File.Info)} callback
 */
Os.File.stat_ = function(file_name, callback) {};
