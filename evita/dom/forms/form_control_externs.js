// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @expose @type{number} */
FormControl.prototype.controlId;

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
