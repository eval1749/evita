// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @type {Array.<!Window>} */
Window.prototype.children;

/** @type {Window} */
Window.prototype.firstChild;

/** @type {number} */
Window.prototype.focusTick_;

/** @type {number} */
Window.prototype.id;

/** @type {Window} */
Window.prototype.lastChild;

/** @type {Window} */
Window.prototype.nextSibling;

/** @type {?Window} */
Window.prototype.parent;

/** @type {Window} */
Window.prototype.previousSibling;

/** @type {string} */
Window.prototype.state;

/** @param {!Window} window */
Window.prototype.appendChild = function(window) {};

/** @param {!Window} window */
Window.prototype.changeParent = function(window) {};

Window.prototype.destroy = function() {};
Window.prototype.focus = function() {};
Window.prototype.realize = function() {};

/** @param {!Window} window */
Window.prototype.removeChild = function(window) {};

/**
 * @param {string} type
 * @param {!EventListener} listener
 * @param {boolean=} opt_capture
 */
Window.prototype.addEventListener = function(type, listener, opt_capture) {};

/**
 * @param {!Event} event
 */
Window.prototype.dispatchEvent = function(event) {};

/**
 * @param {string} type
 * @param {!EventListener} listener
 * @param {boolean=} opt_capture
 */
Window.prototype.removeEventListener = function(type, listener,
                                                opt_capture) {};

/** @param {!Window} new_right_window */
Window.prototype.splitHorizontally = function(new_right_window) {};

/** @param {!Window} new_below_window */
Window.prototype.splitVertically = function(new_below_window) {};
