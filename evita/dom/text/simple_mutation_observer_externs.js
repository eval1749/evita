// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @interface
 */
text.SimpleMutationObserver;

/**
 * @param {number} headCount
 * @param {number} tailCount
 * @param {number} delta
 */
text.SimpleMutationObserver.prototype.didChangeTextDocument = function(
    headCount, tailCount, delta) {};

text.SimpleMutationObserver.prototype.didLoadTextDocument = function() {};

/**
 * @constructor
 * @param {!TextDocument} document
 */
text.SimpleMutationObserverBase = function(document) {};

/** @type {!TextDocument} */
text.SimpleMutationObserverBase.prototype.document;

text.SimpleMutationObserverBase.prototype.didLoadTextDocument = function() {};
text.SimpleMutationObserverBase.prototype.stopObserving = function() {};
