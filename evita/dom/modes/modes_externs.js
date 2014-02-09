// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!Document} document
 */
var Mode = function(document) {};

/** @expose @type {!Document} */
Mode.prototype.document;

/** @expose @type {string} */
Mode.prototype.name;

/** @expose @type {function(number)} */
Mode.prototype.doCall_ = function(number) {};

/**
 * @constructor
 * @extends {Mode}
 * @param {!Document} document
 */
var ConfigMode = function(document) {};

/**
 * @constructor
 * @extends {Mode}
 * @param {!Document} document
 */
var CxxMode = function(document) {};

/**
 * @constructor
 * @extends {Mode}
 * @param {!Document} document
 */
var JavaMode = function(document) {};

/**
 * @constructor
 * @extends {Mode}
 * @param {!Document} document
 */
var HaskellMode = function(document) {};

/**
 * @constructor
 * @extends {Mode}
 * @param {!Document} document
 */
var LispMode = function(document) {};

/**
 * @constructor
 * @extends {Mode}
 * @param {!Document} document
 */
var MasonMode = function(document) {};

/**
 * @constructor
 * @extends {Mode}
 * @param {!Document} document
 */
var PerlMode = function(document) {};

/**
 * @constructor
 * @extends {Mode}
 * @param {!Document} document
 */
var PlainTextMode = function(document) {};

/**
 * @constructor
 * @extends {Mode}
 * @param {!Document} document
 */
var PythonMode = function(document) {};
