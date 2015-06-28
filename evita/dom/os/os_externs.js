// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @export @type{!Object} */
var Os;


/** @export
 *  @constructor
 *  @param {number} winLastError
 */
Os.File.Error = function(winLastError) {};

/**
 * @export
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
 * @export
 * @type {string}
 */
Os.environmentStrings;

/**
 * @export
 * @param {string} name
 * @return {string|undefined}
 */
Os.getenv = function(name) {};
