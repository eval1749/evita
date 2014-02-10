// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @expose @type {function(!Document): !Mode} */
Mode.chooseMode;

/** @expose @type {function(string): !Mode} */
Mode.chooseModeByName;

/** @expose @type {!Mode} */
Mode.defaultMode;

/** @expose @type {!Map.<string, ModeDescription>} */
Mode.extensionMap;

/** @expose @type {!Map.<string, !Function>} */
Mode.nameMap;

/** @expose @type {string} */
Mode.prototype.name;

/** @expose @type {function(number)} */
Mode.prototype.doCall_ = function(number) {};

/**
 * @constructor
 * @extends {Mode}
 */
var ConfigMode = function() {};

/**
 * @constructor
 * @extends {Mode}
 */
var CxxMode = function() {};

/**
 * @constructor
 * @extends {Mode}
 */
var JavaMode = function() {};

/**
 * @constructor
 * @extends {Mode}
 */
var JavaScriptMode = function() {};

/**
 * @constructor
 * @extends {Mode}
 */
var HaskellMode = function() {};

/**
 * @constructor
 * @extends {Mode}
 */
var LispMode = function() {};

/**
 * @constructor
 * @extends {Mode}
 */
var MasonMode = function() {};

/**
 * @constructor
 * @extends {Mode}
 */
var PerlMode = function() {};

/**
 * @constructor
 * @extends {Mode}
 */
var PlainTextMode = function() {};

/**
 * @constructor
 * @extends {Mode}
 */
var PythonMode = function() {};

/**
 * @constructor
 * @extends {Mode}
 */
var XmlMode = function() {};
