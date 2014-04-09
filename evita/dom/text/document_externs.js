// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview Document externs.
 * @externs
 */

/**
 * @enum {number}
 */
Document.Obsolete = {
  NO: 0,
  CHECKING: 1,
  IGNORE: 2,
  UNKNOWN: 3,
  YES: 4
};

/**
 * @typedef {!function(string, !Document)}
 */
Document.Observer;

/**
 * @param {!Document.Observer} observer
 */
Document.addObserver = function(observer) {};

/**
 * @param {string} name
 * @return {?Document}
 */
Document.find = function(name) {};
/**
 * @param {string} absolute_file_name
 * @return {?Document}
 */
Document.findFile = function(absolute_file_name) {};

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

/**
 * @param {!Document.Observer} observer
 */
Document.removeObserver = function(observer) {};

/** @type {number} */
Document.prototype.codePage;

/** @type {string} */
Document.prototype.encoding;

/** @expose  @type {string} */
Document.prototype.filename;

/** @expose  @type {?Keymap} */
Document.prototype.keymap;

/** @type {!Date} */
Document.prototype.lastStatTime_;

/** @type {!Date} */
Document.prototype.lastWriteTime;

/** @expose  @type {number} */
Document.prototype.length;

/** @expose  @type {!Generator.<string>} */
Document.prototype.lines;

/** @expose  @type {!Mode} */
Document.prototype.mode;

/** @expose  @type {boolean} */
Document.prototype.modified;

/** @expose  @type {string} */
Document.prototype.name;

/** @type {number} */
Document.prototype.newline;

/** @expose @type {number} */
Document.prototype.obsolete;

/** @expose  @type {!Map.<string, *>} */
Document.prototype.properties;

/** @expose  @type {boolean} */
Document.prototype.readonly;

/** @type {number} */
Document.prototype.state;

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

/** @type {!function()} */
Document.prototype.clearUndo = function() {};

/** @type {!function()} */
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
 * @param {number} offset
 * @return {{column: number, lineNumber: number}}
 */
Document.prototype.getLineAndColumn_ = function(offset) {};

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
 * @param {!Editor.RegExp} regexp
 * @param {number} start
 * @param {number} end
 * @return {?Array.<!Editor.RegExp.Match>}
 */
Document.prototype.match_ = function(regexp, start, end) {};

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
 * @param {string=} opt_filename
 * @return {!Promise.<number>}
 */
Document.prototype.save = function(opt_filename) {};

/**
 * @param {number} start
 * @param {number=} opt_end
 * @return {string}
 */
Document.prototype.slice = function(start, opt_end) {};

/**
 * @param {string} name
 */
Document.prototype.startUndoGroup_ = function(name) {};

/**
 * @param {!TextOffset} offset
 * @return {number}
 */
Document.prototype.spellingAt = function(offset) {};

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
