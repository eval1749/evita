// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!TextDocument} document
 * @return {undefined}
 */
function TextDocumentState(document) {}

/**
 * @typedef {function(!TextDocument, !TextDocumentState)}
 */
var TextDocumentStateCallback;

/**
 * @param {!TextDocumentStateCallback} callback
 */
TextDocumentState.addObserver = function(callback) {};

/** @type {string} */
TextDocumentState.prototype.fileName;

/** @type {number} */
TextDocumentState.prototype.icon;

/** @type {!Date} */
TextDocumentState.prototype.lastWriteTime;

/** @type {string} */
TextDocumentState.prototype.name;

/** @type {number} */
TextDocumentState.prototype.revision;

/** @type {number} */
TextDocumentState.prototype.state;
