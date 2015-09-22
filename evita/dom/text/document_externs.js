// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview Document externs.
 * @externs
 */

/** @export  @type {!Generator.<string>} */
Document.prototype.lines;

/** @export @type {!Document.Obsolete} */
Document.prototype.obsolete;

/** @export  @type {!Map.<string, *>} */
Document.prototype.properties;

/** @export @type {function()} */
Document.prototype.parseFileProperties;

/**
 * @param {string} name
 * @param {function()} callback
 * @param {!Object=} opt_receiver
 */
Document.prototype.undoGroup = function(name, callback, opt_receiver) {};
