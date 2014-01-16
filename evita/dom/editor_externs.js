// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview Editor
 * @externs
 */

/**
 * @return {Window}
 */
Editor.activeWindow = function() {};

/**
 * @param {?Window} window.
 * @param {!string} filename.
 * @return {Promise.<string>}
 */
Editor.getFilenameForLoad = function(window, filename) {};

/**
 * @param {?Window} window.
 * @param {!string} filename.
 * @param {function(!string)} callback
 */
Editor.getFilenameForLoad_ = function(window, filename, callback) {};

/**
 * @param {?Window} window.
 * @param {!string} filename.
 * @return {Promise.<string>}
 */
Editor.getFilenameForSave = function(window, filename) {};

/**
 * @param {?Window} window.
 * @param {!string} filename.
 * @param {function(!string)} callback
 */
Editor.getFilenameForSave_ = function(window, filename, callback) {};

/**
 * @param {?Window} window.
 * @param {!string} message.
 * @param {!title} title.
 * @param {flags} flags.
 * @return {Promise.<number>}
 */
Editor.messageBox  = function(window, message, title, flags) {};

/**
 * @param {?Window} window.
 * @param {!string} message.
 * @param {!title} title.
 * @param {flags} flags.
 * @param {function(number)} callback
 */
Editor.messageBox_  = function(window, message, title, flags, callback) {};

/**
 * @param {!string} script_text
 * @return {{exception: Error, value: *, stackTrace: Array,
             stackTraceString: string, lineNumber: number, start: number,
             end: number, startColumn: number, endColumn: number}}
 */
Editor.runScript = function(script_text) {};

/**
 * @param {number} key_code
 * @param {function(number=)} command
 */
Editor.setKeyBinding_ = function(key_code, command) {};

/**
 * @const @type{!string}
 */
Editor.version;
