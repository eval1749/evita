// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @type {function() : !TextWindow } */
TextWindow.prototype.clone = function() {};

/**
 * @param {number} method
 * @param {number=} opt_int_or_float1
 * @param {number=} opt_int_or_float2
* @return {number}
 */
TextWindow.prototype.compute_ = function(method, opt_int_or_float1,
                                         opt_int_or_float2) {};

/**
 * @override
 * @type {!TextSelection}
 */
TextWindow.prototype.selection;

/** @type {function()} */
TextWindow.prototype.makeSelectionVisible = function() {};
