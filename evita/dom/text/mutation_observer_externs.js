// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 */
function MutationRecord() {}

/** @type {!Document} */
MutationRecord.prototype.document;

/** @type {number} */
MutationRecord.prototype.offset;

/** @type {string} */
MutationRecord.prototype.type;

/**
 * @typedef {!function(!Array.<!MutationRecord>, !MutationObserver)}
 */
var MutationCallback;

/**
 * @constructor
 * @param {MutationCallback} callback
 */
function MutationObserver(callback) {}

/**
 */
MutationObserver.prototype.disconnect = function() {};

/**
 * @param {!Document} document
 * @param {!MutationObserverInit} options
 */
MutationObserver.prototype.observe = function(document, options) {};

/**
 * @return {!Array.<!MutationRecord>}
 */
MutationObserver.prototype.takeRecords = function() {};

