// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * TODO(yosi) Until Closure compiler supports optional entries in @struct,
 * we use |Object|.
 * @typedef {Object}
 *   bubbles: boolean
 *   cancelable: boolean
 *   detail: number
 *   view: ?Window
 *   altKey: boolean,
 *   button: number,
 *   buttns: number,
 *   ctrlKey: boolean,
 *   metaKey: boolean,
 *   shiftKey: boolean,
 *   deltaMode: number,
 *   deltaX: number,
 *   deltaY: number,
 *   deltaZ: number,
 */
var WheelEventInit;


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
