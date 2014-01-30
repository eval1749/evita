// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @type {function() : !TextWindow } */
TextWindow.prototype.clone = function() {};

/**
 * @param {number} method
 * @param {!Position=} opt_position
 * @param {number=} opt_count
 * @param {Point=} opt_point
* @return {number}
 */
TextWindow.prototype.compute_ = function(method, opt_position, opt_count,
                                         opt_point) {};

/**
 * @override
 * @type {!TextSelection}
 */
TextWindow.prototype.selection;

/** @type {function()} */
TextWindow.prototype.makeSelectionVisible = function() {};

/**
 * @param {number} x
 * @param {number} y
 * @return {!Position}
 */
TextWindow.prototype.mapPointToPosition_ = function(x, y) {};

/**
 * @param {!Position} position
 * @return {!Point}
 */
TextWindow.prototype.mapPositionToPoint_ = function(position) {};
