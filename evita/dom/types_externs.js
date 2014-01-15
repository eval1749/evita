// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview Types
 * @externs
 */

/**
 * @constructor
 */
var Document = function(name) {};
var Range;

/**
 * @constructor
 */
var Editor = function() {};

/**
 * @constructor
 */
var Window = function() {};

/**
 * @constructor
 * @extends {Window}
 */
var EditorWindow = function() {};

var FilePath;

/**
 * @constructor
 * @extends {Window}
 * @param {Range} range.
 */
var TextWindow = function(range) {};

/**
 * @constructor
 * You can't create |Selection| object dirctory. You can obtain |Selection|
 * object via |TextWindow.prototype.selection|.
 */
var Selection = function() {};

/** @typedef {number} */
var Position;

//////////////////////////////////////////////////////////////////////
//
// Unicode
//
var Unicode;

/** @const @type{Array.<string>} */
Unicode.CATEGORY_SHORT_NAMES;

/** @enum{string} */
Unicode.Category = {
  Cc: "Cc", Cf: "Cf", Cn: "Cn", Co: "Co", Cs: "Cs",
  Ll: "Ll", Lm: "Lm", Lo: "Lo", Lt: "Lt", Lu: "Lu", Mc: "Mc"
};

/** @const @type{Array.<{bidi: string, category: string}>} */
Unicode.UCD;
