// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @extends {EventTarget}
 * @param {string} name
 */
function Form(name) {}


/** @expose @type {Array.<!FormControl>} */
Form.prototype.controls;

/** @expose @type {string} */
Form.prototype.name;

/**
 * @param {!FormControl} control
 */
Form.prototype.add = function(control) {};

/**
 * @param {string} type
 * @param {!EventListener} listener
 * @param {boolean=} opt_capture
 */
Form.prototype.addEventListener = function(type, listener, opt_capture) {};

/**
 * @param {number} control_id
 * @return {?FormControl}
 */
Form.prototype.control = function(control_id) {};

/**
 * @param {!Event} event
 */
Form.prototype.dispatchEvent = function(event) {};

/**
 * @param {number} control_id
 * @return {!FormControl}
 */
Form.prototype.get = function(control_id) {};

Form.prototype.realize = function() {};

/**
 * @param {string} type
 * @param {!EventListener} listener
 * @param {boolean=} opt_capture
 */
Form.prototype.removeEventListener = function(type, listener, opt_capture) {};

Form.prototype.show = function() {};
