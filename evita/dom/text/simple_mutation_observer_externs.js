// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @interface
 */
text.SimpleMutationObserver;

/**
 * @param {number} offset
 */
text.SimpleMutationObserver.prototype.didChangeDocument = function(offset) {};

/**
 * @param {number} offset
 */
text.SimpleMutationObserver.prototype.didLoadDocument = function(offset) {};

/**
 * @constructor
 * @param {!Document} document
 */
text.SimpleMutationObserverBase = function(document) {};

/** @type {!Document} */
text.SimpleMutationObserverBase.prototype.document;

text.SimpleMutationObserverBase.prototype.didLoadDocument = function() {};
text.SimpleMutationObserverBase.prototype.stopObserving = function() {};
