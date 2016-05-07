// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('visuals');

goog.scope(function() {
/**
 * @interface
 */
var DOMTokenListOwner = function() {};

/** @param {string} token */
DOMTokenListOwner.didAddToken = function(token) {};

/** @param {string} token */
DOMTokenListOwner.didRemoveToken = function(token) {};

/** @interface */
visuals.DOMTokenListOwner = DOMTokenListOwner;
});
