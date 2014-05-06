// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @type {function(): number} */
TextPosition.prototype.charCode = function() {};

/** @type {function(): string} */
TextPosition.prototype.charSyntax = function() {};

/** @type {!Document} */
TextPosition.prototype.document;

/** @type {number} */
TextPosition.prototype.offset;

/**
 * @param {!Unit} unit
 * @param {number=} opt_count, default is one.
 */
TextPosition.prototype.move = function(unit, opt_count) {};

/**
 * @param {function() : boolean} callback
 * @param {number=} opt_count, default is one.
 * @return {!TextPosition}
 */
TextPosition.prototype.moveWhile = function(callback, opt_count) {};
