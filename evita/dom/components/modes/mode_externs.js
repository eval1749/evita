// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @const @type {!Object} */
var modes;

/**
 * @constructor
 */
modes.Mode = function() {};

/**
 * @param {!TextDocument} document
 */
modes.Mode.prototype.attach = function(document) {};

modes.Mode.prototype.detach = function() {};

/** @type {string} */
modes.Mode.description;

/** @type {!TextDocument} */
modes.Mode.document;

/** @type {string} */
modes.Mode.id;

/** @type {!Map} */
modes.Mode.keymap;

/** @type {string} */
modes.Mode.name;

/**
 * @param {!TextDocument} document
 * @param {number} hint
 */
modes.Mode.prototype.doColor = function(document, hint) {};

/**
 * @param {!TextDocument} document
 * @return {!modes.Mode}
 */
modes.Mode.chooseMode = function(document) {};

/**
 * @param {string} fileName
 * @return {!modes.Mode}
 */
modes.Mode.chooseModeByFileName = function(fileName) {};

/**
 * @param {string} id
 * @param {string} modeId
 * @param {string} text
 *
 */
modes.Mode.registerExtension = function(id, modeId, text) {};

/**
 * @param {string} fileName
 * @param {string} modeId
 */
modes.Mode.registerFileName = function(fileName, modeId) {};

/**
 * @param {string} id
 * @param {string} name
 */
modes.Mode.registerMode = function(id, name) {};

/** @typedef {modes.Mode} */
var Mode;
