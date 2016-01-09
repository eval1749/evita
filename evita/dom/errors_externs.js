// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!TextDocument} document
 * @param {string=} opt_message
 */
var TextDocumentError = function(document, opt_message) {};

/** @export @type {!TextDocument} */
TextDocumentError.prototype.document;

/**
 * @constructor
 * @extends {TextDocumentError}
 * @param {!TextDocument} document
 * @param {string=} opt_message
 */
var TextDocumentNotReady = function(document, opt_message) {};

/**
 * @constructor
 * @extends {TextDocumentError}
 * @param {!TextDocument} document
 * @param {string=} opt_message
 */
var TextDocumentReadOnly = function(document, opt_message) {};

