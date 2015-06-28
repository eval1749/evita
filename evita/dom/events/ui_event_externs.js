// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @export @type{number} */
UiEvent.prototype.detail;

/** @export @type{?Window} */
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
