// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @expose @type{!Window} */
WindowEvent.prototype.sourceWindow;

/**
 * @expose
 * @param {string} type
 * @param {boolean} bubbles
 * @param {boolean} cancelable
 * @param {!Window} source_window
 */
WindowEvent.prototype.initWindowEvent = function(type, bubbles, cancelable,
                                                 source_window) {};
