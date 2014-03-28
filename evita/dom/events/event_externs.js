// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @expose @type{boolean} */
Event.prototype.bubbles;

/** @expose @type{boolean} */
Event.prototype.cancelable;

/** @expose @type{?EventTarget} */
Event.prototype.current_target;

/** @expose @type{boolean} */
Event.prototype.defaultPrevented;

/** @expose @type{Event.PhaseType} */
Event.prototype.eventPhase;

/** @expose @type{number} */
Event.prototype.timeStamp;

/** @expose @type{?EventTarget} */
Event.prototype.target;

/** @expose @type{string} */
Event.prototype.type;

/**
 * @param {string} type
 * @param {boolean} bubbles
 * @param {boolean} cancelable
 */
Event.prototype.initEvent = function(type, bubbles, cancelable) {};
