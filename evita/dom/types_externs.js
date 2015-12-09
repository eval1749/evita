// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview Types
 * @externs
 */

/**
 * TODO(eval1749): Once closure compiler has build-in externs for |Array.from()|
 * we should remove our own type annotation fro |Array.from()|.
 * @template T, U
 * @param {!Iterable<T>} arrayLike
 * @param {!function(T):U=} opt_mapFn
 * @param {*=} opt_thisArg
 * TODO(eval1749): |Array.from()| should return |!Array.<U>|.
 * @return {!Array.<T|U>}
 */
Array.from = function(arrayLike, opt_mapFn, opt_thisArg) {};

/**
 * @type {!Object}
 */
var global;

/** @typedef {!Function|!Object} Object which has |handleEvent(event)| */
var EventListener;

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

// TODO(eval1749): We should rename |Range.Case| to |CaseAnalysisResult|.
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

/**
 * @interface
 */
var Runnable;

/** @typedef {number} */
var TextOffset;

/**
 * @constructor
 * @param {!Document} document
 * @param {!TextOffset} offset
 */
var TextPosition = function(document, offset) {};

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

/** @typedef {symbol} */
Unicode.Script;

/**
 * @typedef {{
 *    bidi: string,
 *    category: string,
 *    script: Unicode.Script
 * }}
 */
Unicode.CharacterData;

/** @const @type{Array.<!Unicode.CharacterData>} */
Unicode.UCD;

// TODO(eval1749): Once Closure compiler works with |new RegExp(...)|, we should
// remove |NewRegExp|. V20140508 doesn't work.
/**
 * @param {string} source
 * @param {string=} opt_flags
 * @return {!RegExp}
 */
function NewRegExp(source, opt_flags) {}
