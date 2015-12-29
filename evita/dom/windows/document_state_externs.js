// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!Document|!DocumentState} document_or_state
 * @return {undefined}
 */
function DocumentState(document_or_state) {}

/**
 * @typedef {function(!Document, !DocumentState)}
 */
var DocumentStateCallback;

/**
 * @param {!DocumentStateCallback} callback
 */
DocumentState.addObserver = function(callback) {};

/**
 * @param {!Document} document
 */
DocumentState.update = function(document) {};

/** @type {string} */
DocumentState.prototype.fileName;

/** @type {number} */
DocumentState.prototype.icon;

/** @type {!Date} */
DocumentState.prototype.lastWriteTime;

/** @type {string} */
DocumentState.prototype.name;

/** @type {number} */
DocumentState.prototype.state;

/** @return {!DocumentState} */
DocumentState.prototype.clone = function() {};

/**
 * @param {!DocumentState} other
 * @return {boolean}
 */
DocumentState.prototype.equals = function(other) {};
