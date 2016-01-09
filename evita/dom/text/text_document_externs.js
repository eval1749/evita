// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview TextDocument externs.
 * @externs
 */

/** @export  @type {!Generator.<string>} */
TextDocument.prototype.lines;

/** @export @type {!TextDocument.Obsolete} */
TextDocument.prototype.obsolete;

/** @export  @type {!Map.<string, *>} */
TextDocument.prototype.properties;

/** @export @type {function()} */
TextDocument.prototype.parseFileProperties;

/**
 * @param {string} name
 * @param {function()} callback
 * @param {!Object=} opt_receiver
 */
TextDocument.prototype.undoGroup = function(name, callback, opt_receiver) {};
