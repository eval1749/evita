// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 */
function TextEncodeOptions() {}

/** @type {boolean} */
TextEncodeOptions.prototype.stream;

/**
 * @constructor
 * @param {string=} opt_label
 */
function TextEncoder(opt_label) {}

/**
 * @param {string=} opt_input
 * @param {!TextEncodeOptions=} opt_options
 * @return {!Uint8Array}
 */
TextEncoder.prototype.encode = function(opt_input, opt_options) {};
