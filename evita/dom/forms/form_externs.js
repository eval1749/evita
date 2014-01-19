// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @expose @type {Array.<!FormControl>} */
Form.prototype.controls;

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
 * @param {!Event} event
 */
Form.prototype.dispatchEvent = function(event) {};

Form.prototype.realize = function() {};

/**
 * @param {string} type
 * @param {!EventListener} listener
 * @param {boolean=} opt_capture
 */
Form.prototype.removeEventListener = function(type, listener, opt_capture) {};

Form.prototype.show = function() {};
