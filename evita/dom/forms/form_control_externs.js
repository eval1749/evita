// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @expose @type{number} */
FormControl.prototype.clientHeight;

/** @expose @type{number} */
FormControl.prototype.clientLeft;

/** @expose @type{number} */
FormControl.prototype.clientTop;

/** @expose @type{number} */
FormControl.prototype.clientWidth;

/** @expose @type{number} */
FormControl.prototype.controlId;

/** @expose @type {boolean} */
FormControl.prototype.disabled;

/** @expose @type {?Form} */
FormControl.prototype.form;

/** @expose @type {string} */
FormControl.prototype.name;

/**
 * @param {string} type
 * @param {!EventListener} listener
 * @param {boolean=} opt_capture
 */
FormControl.prototype.addEventListener = function(type, listener,
                                                  opt_capture) {};

/**
 * @param {!Event} event
 */
FormControl.prototype.dispatchEvent = function(event) {};

/**
 * @param {string} type
 * @param {!EventListener} listener
 * @param {boolean=} opt_capture
 */
FormControl.prototype.removeEventListener = function(type, listener,
                                                     opt_capture) {};
