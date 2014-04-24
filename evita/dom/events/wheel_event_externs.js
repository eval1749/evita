// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @extends {MouseEvent}
 * @param {string} type
 * @param {WheelEventInit=} opt_init_dict
 */
var WheelEvent = function(type, opt_init_dict) {};

/** @type {number} */
WheelEvent.prototype.deltaMode;

/** @type {number} */
WheelEvent.prototype.deltaX;

/** @type {number} */
WheelEvent.prototype.deltaY;

/** @type {number} */
WheelEvent.prototype.deltaZ;
