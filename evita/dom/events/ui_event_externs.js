// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @expose @type{number} */
UiEvent.prototype.detail;

/** @expose @type{?Window} */
UiEvent.prototype.view;

/**
 * @param {string} type
 * @param {boolean} bubbles
 * @param {boolean} cancelable
 * @param {?Window} view
 * @param {number} detail
 */
UiEvent.prototype.initUiEvent = function(type, bubbles, cancelable,
                                         view, detail) {};
