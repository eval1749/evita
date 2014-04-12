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

Editor.exit = function() {};
Editor.forceExit = function() {};

/**
 * @param {Function} window_class
 * @param {string} key_combination
 * @param {function(number=)} command
 * @param {string=} opt_description
 */
Editor.bindKey = function(window_class, key_combination, command,
                          opt_description) {};

/**
 * @param {string} word_to_check
 * @return {!Promise.<boolean>}
 */
Editor.checkSpelling = function(word_to_check) {};

/**
 * @param {?Window} window.
 * @param {!string} filename.
 * @return {Promise.<string>}
 */
Editor.getFilenameForLoad = function(window, filename) {};

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
 * @param {string} wrong_word
 * @return {!Promise.<Array.<string>>}
 */
Editor.GetSpellingSuggestions = function(wrong_word) {};

/** @type {Map.<string, Map.<number,function(number=)>>} */
Editor.key_binding_map_map_;

/**
 * @param {string} format_text
 * @param {Object.<string, string>=} opt_dict
 * @return {string}
 */
Editor.localizeText = function(format_text, opt_dict) {};

/**
 * @param {?Window} window
 * @param {string} message
 * @param {number} flags
 * @param {string=} opt_title
 * @return {!Promise.<number>}
 */
Editor.messageBox = function(window, message, flags, opt_title) {};

/**
 * @param {?Window} window
 * @param {!string} message
 * @param {number} flags
 * @param {string} title
 * @param {!function(number)} callback
 */
Editor.messageBox_ = function(window, message, flags, title, callback) {};

/**
 * @param {string} name
 * @return {string}
 */
Editor.metrics = function(name) {};

/**
 * @param {string} spec
 * @return {number} key code.
*/
Editor.parseKeyCombination = function(spec) {};

/**
 * @param {!Window} window
 * @param {!TabData} tabData
 */
Editor.setTabData = function(window, tabData) {};

/**
 * @typedef {{
 *  column: number,
 *  functionName: string,
 *  lineNumber: number,
 *  scriptName: string
 * }}
 */
var StackFrame;

/**
 * @param {string} script_text
 * @param {string=} opt_file_name
 * @return {{exception: Error, value: *, stackTrace: Array.<!StackFrame>,
             stackTraceString: string, lineNumber: number, start: number,
             end: number, startColumn: number, endColumn: number}}
 */
Editor.runScript = function(script_text, opt_file_name) {};

/**
 * @const @type{!string}
 */
Editor.version;
