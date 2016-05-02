// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview Types
 * @externs
 */

/*
 * TODO(eval1749): Once closure compiler has build-in externs for |Array.from()|
 * we should remove our own type annotation fro |Array.from()|.
 * $template T, U
 * $param {!Iterable<T>} arrayLike
 * $param {!function(T):U=} opt_mapFn
 * $param {*=} opt_thisArg
 * TODO(eval1749): |Array.from()| should return |!Array.<U>|.
 * $return {!Array.<T|U>}
 */
// Array.from = function(arrayLike, opt_mapFn, opt_thisArg) {};

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

/**
 * @interface
 */
var Runnable;

/** @typedef {number} */
var TextOffset;

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

/**
 * @const @type {!Array<string>}
 * The order of Unicode category name must be matched with ICU.
 * See |UCharCategory| in "icu/source/common/unicode/uchar.h"
 *
 */
Unicode.CATEGORY_SHORT_NAMES;

/**
 * @const @type {!Array<string>}
 * The order of Unicode category name must be matched with ICU.
 * See |UCharCategory| in "icu/source/common/unicode/uchar.h"
 *
 */
Unicode.SCRIPT_NAMES;

/** @enum{string} */
Unicode.Category = {
  Cc: 'Cc',
  Cf: 'Cf',
  Cn: 'Cn',
  Co: 'Co',
  Cs: 'Cs',
  Ll: 'Ll',
  Lm: 'Lm',
  Lo: 'Lo',
  Lt: 'Lt',
  Lu: 'Lu',
  Mc: 'Mc',
  Me: 'Me',
  Mn: 'Mn',
  Nd: 'Nd',
  Nl: 'Nl',
  No: 'No',
  Pc: 'Pc',
  Pd: 'Pd',
  Pe: 'Pe',
  Pf: 'Pf',
  Pi: 'Pi',
  Po: 'Po',
  Ps: 'Ps',
  Sc: 'Sc',
  Sk: 'Sk',
  Sm: 'Sm',
  So: 'So',
  Zl: 'Zl',
  Zp: 'Zp',
  Zs: 'Zs',
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

/** @const @type {!repl.Console} */
var console;

/** @type {!Map<string, *>} */
TextDocument.prototype.properties;

/** @const @type {!Object} */
var Os;
