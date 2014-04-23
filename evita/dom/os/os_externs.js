// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @expose @type{!Object} */
var Os;


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
