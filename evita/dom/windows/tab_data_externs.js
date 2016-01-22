// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @return {undefined}
 */
function TabData() {}

/**
 * @param {!TextWindow} window
 * @param {!TextDocumentState} state
 */
TabData.update = function(window, state) {};

/** @type {number} */
TabData.prototype.icon;

/** @type {number} */
TabData.prototype.state;

/** @type {string} */
TabData.prototype.title;

/** @type {string} */
TabData.prototype.tooltip;
