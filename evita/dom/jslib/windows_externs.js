// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @type {!Object} */
var windows;

/**
 * @param {!Window} window
 * @param {!Document} document
 * @return {!TextWindow}
 */
windows.activate = function(window, document) {};

/**
 * @return {?Window}
 */
windows.firstWindow = function() {};

/**
 * @return {?Window}
 */
windows.lastWindow = function() {};

/**
 * @param {!Document} document
 */
windows.newEditorWindow = function(document) {};

/**
 * @param {!Window} window
 * @param {!Document} document
 * @return {!TextWindow}
 */
windows.newTextWindow = function(window, document) {};

/**
 * @param {!Window} current
 * @return {?Window}
 */
windows.nextWindow = function(current) {};

/**
 * @param {!Window} current
 * @return {?Window}
 */
windows.previousWindow = function(current) {};
