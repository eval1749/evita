// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview EventTarget
 * @externs
 */

/**
 * @param {string} type
 * @param {EventListener} listener
 * @param {boolean=} opt_capture
 */
EventTarget.prototype.addEventListener = function(type, listener,
                                                  opt_capture) {};

/**
 * @param {Event} event
 */
EventTarget.prototype.dispatchEvent = function(event) {};

/**
 * @param {string} type
 * @param {EventListener} listener
 * @param {boolean=} opt_capture
 */
EventTarget.prototype.removeEventListener = function(type, listener,
                                                     opt_capture) {};
