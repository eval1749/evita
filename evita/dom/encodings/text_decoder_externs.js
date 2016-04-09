// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {string=} opt_label
 * @param {!TextDecoderOptions=} opt_options
 */
function TextDecoder(opt_label, opt_options) {}

/**
 * @type {string}
 */
TextDecoder.prototype.encoding;

/**
 * @param {!ArrayBufferView=} opt_input
 * @param {!TextDecodeOptions=} opt_options
 * @return {string}
 */
TextDecoder.prototype.decode = function(opt_input, opt_options) {};
