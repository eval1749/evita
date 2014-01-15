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
Editor.getFilenameForSave = function(window, filename) {};

/**
 * @param {?Window} window.
 * @param {!string} message.
 * @param {!title} title.
 * @param {flags} flags.
 * @return {Promise.<number>}
 */
Editor.messageBox  = function(window, message, title, flags) {};
