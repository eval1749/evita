// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @type {function() : !TextWindow } */
TextWindow.prototype.clone = function() {};

/**
 * @override
 * @type {!TextSelection}
 */
TextWindow.prototype.selection;

/**
 * @param {number} position
 * @return {number}
 */
TextWindow.prototype.endOfLine_ = function(position) {};

/** @type {function()} */
TextWindow.prototype.makeSelectionVisible = function() {};
