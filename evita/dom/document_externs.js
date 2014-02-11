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
 * @param {!Mode=} opt_mode
 * @return {!Document}
 */
Document.getOrNew = function(name, opt_mode) {};

/**
 * @type {Array.<!Document>}
 */
Document.list;

/**
 * @param {string} filename
 * @return {!Document}
 */
Document.open = function(filename) {};

/**
 * @param {!Document} document
 */
Document.remove = function(document) {};

/** @expose  @type {string} */
Document.prototype.filename;

/** @expose  @type {?Keymap} */
Document.prototype.keymap;

/** @expose  @type {number} */
Document.prototype.length;

/** @expose  @type {!Mode} */
Document.prototype.mode;

/** @expose  @type {boolean} */
Document.prototype.modified;

/** @expose  @type {string} */
Document.prototype.name;

/** @expose  @type {!Map.<string, *>} */
Document.prototype.properties;

/** @expose  @type {boolean} */
Document.prototype.readonly;

/**
 * @param {string} key_combination.
 * @param {Object} command.
 */
Document.prototype.bindKey = function (key_combination, command) {};

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

/** @param {number} hint */
Document.prototype.doColor_ = function(hint) {};

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
 * @param {string=} opt_filename
* @return {!Promise.<number>}
*/
Document.prototype.load = function(opt_filename) {};

/**
 * @param {string} filename
 * @param {function(number)} callback
 */
Document.prototype.load_ = function(filename, callback) {};

/**
 * @return {boolean}.
 */
Document.prototype.needSave = function() {};

/** @expose @type {function()} */
Document.prototype.parseFileProperties;

/** @param {!TextOffset} position */
Document.prototype.redo = function(position) {}

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
