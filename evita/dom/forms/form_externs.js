// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @extends {EventTarget}
 */
function Form() {}

/** @expose @type {Array.<!FormControl>} */
Form.prototype.controls;

/** @expose @type {?FormControl} */
Form.prototype.focusControl;

/** @expose @type {number} */
Form.prototype.height;

/** @expose @type {string} */
Form.prototype.title;

/** @expose @type {number} */
Form.prototype.width;

/**
 * @param {!FormControl} control
 */
Form.prototype.add = function(control) {};
