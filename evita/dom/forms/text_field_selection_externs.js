// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 */
function TextFieldSelection() {}

/** @type {number} */
TextFieldSelection.prototype.end;

/** @type {number} */
TextFieldSelection.prototype.start;

/** @type {boolean} */
TextFieldSelection.prototype.startIsActive;

/**
 * @param {number} start
 * @param {number} end
 * @param {boolean=} opt_start_is_active
 */
TextFieldSelection.prototype.setRange =
    function(start, end, opt_start_is_active) {};
