// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @export @type{boolean} */
Event.prototype.bubbles;

/** @export @type{boolean} */
Event.prototype.cancelable;

/** @export @type{?EventTarget} */
Event.prototype.currentTarget;

/** @export @type{boolean} */
Event.prototype.defaultPrevented;

/** @export @type{Event.PhaseType} */
Event.prototype.eventPhase;

/** @export @type{number} */
Event.prototype.timeStamp;

/** @export @type{?EventTarget} */
Event.prototype.target;

/** @export @type{string} */
Event.prototype.type;

/**
 * @param {string} type
 * @param {boolean} bubbles
 * @param {boolean} cancelable
 */
Event.prototype.initEvent = function(type, bubbles, cancelable) {};

/** @type {function()} */
Event.prototype.preventDefault = function() {}

/** @type {function()} */
Event.prototype.stopImmediatePropagation = function() {}

/** @type {function()} */
Event.prototype.stopPropagation = function() {}
