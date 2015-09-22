// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview Types
 * @externs
 */

/**
 * @type {Object.<string>}
 */
var global = {};

/** @typedef {!Function|!Object} Object which has |handleEvent(event)| */
var EventListener;

var FilePath;

/**
 * Note: Since, Closure compiler doesn't support recursive type, we use
 * |!Map| instead of |!Keymap|.
 * @typedef {Map.<number, !Function|!Map>}
 */
var Keymap;

/**
 * @typedef {RegularExpression}
 */
Editor.RegExp;

/**
 * @typedef {{name: string, start: number, end: number}}
 */
Editor.RegExp.Match;

/**
 * @typedef {{name: string, start: number, end: number}}
 */
var RegExpMatch;

// TODO(eval1749) We should rename |Range.Case| to |CaseAnalysisResult|.
/** @enum {!symbol} */
Range.Case = {
  // "This is capitalized."
  CAPITALIZED_TEXT: 'CAPITALIZED_TEXT',
  // "This Is Capitalized Words."
  CAPITALIZED_WORDS: 'CAPITALIZED_WORDS',
  // "this is lower."
  LOWER: 'LOWER',
  // "ThisIsMixed."
  MIXED: 'MIXED',
  // "THIS IS UPPER."
  UPPER: 'UPPER',
};

/** @typedef {number} */
var TextOffset;

/**
 * @constructor
 * @param {!Document} document
 * @param {!TextOffset} offset
 */
var TextPosition = function(document, offset) {};

/**
 * @constructor
 */
var Style = function() {};

//////////////////////////////////////////////////////////////////////
//
// Editor.idl
//
/**
 * @typedef {Map.<string, Map.<number,function(number=)>>}
 */
var KeyBindingMapMap;

/**
 * @typedef {{
 *  column: number,
 *  functionName: string,
 *  lineNumber: number,
 *  scriptName: string
 * }}
 */
var StackFrame;

/**
 * @typedef {Object.<string, string>}
 */
var StringDict;

/**
 * @typedef {{
 *  exception: Error,
 *  value: *,
 *  stackTrace: Array.<!StackFrame>,
 *  stackTraceString: string,
 *  lineNumber: number,
 *  start: number,
 *  end: number,
 *  startColumn: number,
 *  endColumn: number
 * }}
 */
var RunScriptResult;

/** @typedef {boolean|number|string} */
var SwitchValue;

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

// TODO(yosi) Once Closure compiler works with |new RegExp(...)|, we should
// remove |NewRegExp|. V20140508 doesn't work.
/**
 * @param {string} source
 * @param {string=} opt_flags
 * @return {!RegExp}
 */
function NewRegExp(source, opt_flags) {}
