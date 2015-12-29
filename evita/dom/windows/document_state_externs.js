// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!Document} document
 * @return {undefined}
 */
function DocumentState(document) {}

/**
 * @typedef {function(!Document, !DocumentState)}
 */
var DocumentStateCallback;

/**
 * @param {!DocumentStateCallback} callback
 */
DocumentState.addObserver = function(callback) {};

/** @type {string} */
DocumentState.prototype.fileName;

/** @type {number} */
DocumentState.prototype.icon;

/** @type {!Date} */
DocumentState.prototype.lastWriteTime;

/** @type {string} */
DocumentState.prototype.name;

/** @type {number} */
DocumentState.prototype.revision;

/** @type {number} */
DocumentState.prototype.state;
