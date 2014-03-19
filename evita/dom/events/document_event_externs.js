// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!Object} init_dict
 *  errorCode: number
 */
var DocumentEventInit = function(init_dict) {};

/**
 * @constructor
 * @extends {Event}
 * @param {string} type
 * @param {!DocumentEventInit=} opt_init
 */
var DocumentEvent = function(type, opt_init) {};

/** @type {?Window} */
DocumentEvent.prototype.view;
