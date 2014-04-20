// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @constructor
 * @extends {EventTarget}
 */
var FormControl = function() {};

/** @expose @type{string} */
FormControl.prototype.accessKey;

/** @expose @type{number} */
FormControl.prototype.clientHeight;

/** @expose @type{number} */
FormControl.prototype.clientLeft;

/** @expose @type{number} */
FormControl.prototype.clientTop;

/** @expose @type{number} */
FormControl.prototype.clientWidth;

/** @expose @type {boolean} */
FormControl.prototype.disabled;

/** @expose @type {?Form} */
FormControl.prototype.form;

/** @expose @type {string} */
FormControl.prototype.name;

/**
 * @return {boolean}
 */
FormControl.prototype.canFocus = function() {};

/** @type {!function()} */
FormControl.prototype.focus = function() {};
