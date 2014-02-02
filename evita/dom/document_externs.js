// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview Document externs.
 * @externs
 */

/**
 * @param {string} name
 * @return {?Document}
 */
Document.find = function(name) {};

/**
 * @param {string} name
 * @return {!Document}
 */
Document.getOrNew = function(name) {};

/**
 * @type {Array.<!Document>}
 */
Document.list;

/**
 * @param {string} filename
 */
Document.load = function(filename) {};

/**
 * @param {!Document} document
 */
Document.remove = function(document) {};

/** @expose  @type {string} */
Document.prototype.filename;

/** @expose  @type {number} */
Document.prototype.length;

/** @expose  @type {boolean} */
Document.prototype.modified;

/** @expose  @type {string} */
Document.prototype.name;

/** @expose  @type {!Map.<string, *>} */
Document.prototype.properties;

/**
 * @param {string} key_combination.
 * @param {Object} command.
 */
Document.prototype.bindKey = function (key_combination, command) {};

/**
 * @param {number} key_code
 * @param {function(number?)} command
 */
Document.prototype.bindKey_ = function(key_code, command) {};

/**
 * @param {number} index
 * @return {number}
 */
Document.prototype.charCodeAt_ = function(index) {};

Document.prototype.close = function() {};

/**
 * @param {Unit} unit.
 * @param {number} position.
 * @return {number} new position.
 */
Document.prototype.computeEndOf_ = function(unit, position) {};

/**
 * @param {Unit} unit.
 * @param {number} count.
 * @param {number} position.
 * @return {number}
 */
Document.prototype.computeMotion_ = function(unit, count, position) {};

/**
 * @param {Unit} unit.
 * @param {number} position.
 * @return {number} new position.
 */
Document.prototype.computeStartOf_ = function(unit, position) {};

/**
 * @param {string} char_set
 * @param {number} count
 * @param {number} position
 * @return {number}
 */
Document.prototype.computeWhile_ = function(char_set, count, position) {};

/**
 * @param {string} name
 */
Document.prototype.endUndoGroup_ = function(name) {};

Document.prototype.forceClose = function() {};

/**
 * @return {Array.<!DocumentWindow>}
 */
Document.prototype.listWindows = function() {};

/**
 * @param {string} filename
 */
Document.prototype.load = function(filename) {};

/**
 * @param {string} filename
 */
Document.prototype.load_ = function(filename) {};

/**
 * @return {boolean}.
 */
Document.prototype.needSave = function() {};

/** @param {!TextOffset} position */
Document.prototype.redo= function(position) {}

/** @type {function()} */
Document.prototype.reload_ = function(document) {};

/** @param {string} new_name */
Document.prototype.renameTo = function(new_name) {}

/**
 * @param {string} filename.
 */
Document.prototype.save = function(filename) {};

/**
 * @param {string} name
 */
Document.prototype.startUndoGroup_ = function(name) {};

/**
 * @param {!TextOffset} position
 * @return {!Style}
 */
Document.prototype.styleAt = function(position) {};

/** @param {!TextOffset} position */
Document.prototype.undo = function(position) {}

/**
 * @param {string} name
 * @param {function()} callback
 * @param {!Object=} opt_receiver
 */
Document.prototype.undoGroup = function(name, callback, opt_receiver) {};
