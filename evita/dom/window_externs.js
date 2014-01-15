// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @type {Array.<Window>} */
Window.prototype.children;

/** @type {number} */
Window.prototype.focusTick_;

/** @type {number} */
Window.prototype.id;

/** @type {?Window} */
Window.prototype.parent;

/** @type {!string} */
Window.prototype.state;

/** @param {!Window} window */
Window.prototype.add = function(window) {};

/** @param {!Window} window */
Window.prototype.changeParent = function(window) {};

Window.prototype.destroy = function() {};
Window.prototype.focus = function() {};
Window.prototype.realize = function() {};

/** @param {!Window} window */
Window.prototype.remove= function(window) {};

