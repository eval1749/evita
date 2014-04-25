// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @type {boolean} */
Timer.prototype.isRunning;

/** @type {function()} */
Timer.prototype.stop = function() {};

/**
 * @param {number} delayMs
 * @param {!function()} callback
 * @param {*=} opt_receiver
 */
Timer.prototype.start = function(delayMs, callback, opt_receiver) {};
