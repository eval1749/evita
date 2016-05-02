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

/** @enum{number} */
Unicode.Script = {
  COMMON: 0,
  INHERITED: 1,
  ARABIC: 2,
  ARMENIAN: 3,
  BENGALI: 4,
  BOPOMOFO: 5,
  CHEROKEE: 6,
  COPTIC: 7,
  CYRILLIC: 8,
  DESERET: 9,
  DEVANAGARI: 10,
  ETHIOPIC: 11,
  GEORGIAN: 12,
  GOTHIC: 13,
  GREEK: 14,
  GUJARATI: 15,
  GURMUKHI: 16,
  HAN: 17,
  HANGUL: 18,
  HEBREW: 19,
  HIRAGANA: 20,
  KANNADA: 21,
  KATAKANA: 22,
  KHMER: 23,
  LAO: 24,
  LATIN: 25,
  MALAYALAM: 26,
  MONGOLIAN: 27,
  MYANMAR: 28,
  OGHAM: 29,
  OLD_ITALIC: 30,
  ORIYA: 31,
  RUNIC: 32,
  SINHALA: 33,
  SYRIAC: 34,
  TAMIL: 35,
  TELUGU: 36,
  THAANA: 37,
  THAI: 38,
  TIBETAN: 39,
  CANADIAN_ABORIGINAL: 40,
  YI: 41,
  TAGALOG: 42,
  HANUNOO: 43,
  BUHID: 44,
  TAGBANWA: 45,
  BRAILLE: 46,
  CYPRIOT: 47,
  LIMBU: 48,
  LINEAR_B: 49,
  OSMANYA: 50,
  SHAVIAN: 51,
  TAI_LE: 52,
  UGARITIC: 53,
  KATAKANA_OR_HIRAGANA: 54,
  BUGINESE: 55,
  GLAGOLITIC: 56,
  KHAROSHTHI: 57,
  SYLOTI_NAGRI: 58,
  NEW_TAI_LUE: 59,
  TIFINAGH: 60,
  OLD_PERSIAN: 61,
  BALINESE: 62,
  BATAK: 63,
  BLISSYMBOLS: 64,
  BRAHMI: 65,
  CHAM: 66,
  CIRTH: 67,
  OLD_CHURCH_SLAVONIC_CYRILLIC: 68,
  DEMOTIC_EGYPTIAN: 69,
  HIERATIC_EGYPTIAN: 70,
  EGYPTIAN_HIEROGLYPHS: 71,
  KHUTSURI: 72,
  SIMPLIFIED_HAN: 73,
  TRADITIONAL_HAN: 74,
  PAHAWH_HMONG: 75,
  OLD_HUNGARIAN: 76,
  HARAPPAN_INDUS: 77,
  JAVANESE: 78,
  KAYAH_LI: 79,
  LATIN_FRAKTUR: 80,
  LATIN_GAELIC: 81,
  LEPCHA: 82,
  LINEAR_A: 83,
  MANDAIC: 84,
  MAYAN_HIEROGLYPHS: 85,
  MEROITIC_HIEROGLYPHS: 86,
  NKO: 87,
  ORKHON: 88,
  OLD_PERMIC: 89,
  PHAGS_PA: 90,
  PHOENICIAN: 91,
  MIAO: 92,
  RONGORONGO: 93,
  SARATI: 94,
  ESTRANGELO_SYRIAC: 95,
  WESTERN_SYRIAC: 96,
  EASTERN_SYRIAC: 97,
  TENGWAR: 98,
  VAI: 99,
  VISIBLE_SPEECH: 100,
  CUNEIFORM: 101,
  UNWRITTEN_LANGUAGES: 102,
  UNKNOWN: 103,
  CARIAN: 104,
  JAPANESE: 105,
  LANNA: 106,
  LYCIAN: 107,
  LYDIAN: 108,
  OL_CHIKI: 109,
  REJANG: 110,
  SAURASHTRA: 111,
  SIGN_WRITING: 112,
  SUNDANESE: 113,
  MOON: 114,
  MEITEI_MAYEK: 115,
  IMPERIAL_ARAMAIC: 116,
  AVESTAN: 117,
  CHAKMA: 118,
  KOREAN: 119,
  KAITHI: 120,
  MANICHAEAN: 121,
  INSCRIPTIONAL_PAHLAVI: 122,
  PSALTER_PAHLAVI: 123,
  BOOK_PAHLAVI: 124,
  INSCRIPTIONAL_PARTHIAN: 125,
  SAMARITAN: 126,
  TAI_VIET: 127,
  MATHEMATICAL_NOTATION: 128,
  SYMBOLS: 129,
  BAMUM: 130,
  LISU: 131,
  NAKHI_GEBA: 132,
  OLD_SOUTH_ARABIAN: 133,
  BASSA_VAH: 134,
  DUPLOYAN: 135,
  ELBASAN: 136,
  GRANTHA: 137,
  KPELLE: 138,
  LOMA: 139,
  MENDE: 140,
  MEROITIC_CURSIVE: 141,
  OLD_NORTH_ARABIAN: 142,
  NABATAEAN: 143,
  PALMYRENE: 144,
  KHUDAWADI: 145,
  WARANG_CITI: 146,
  AFAKA: 147,
  JURCHEN: 148,
  MRO: 149,
  NUSHU: 150,
  SHARADA: 151,
  SORA_SOMPENG: 152,
  TAKRI: 153,
  TANGUT: 154,
  WOLEAI: 155,
  ANATOLIAN_HIEROGLYPHS: 156,
  KHOJKI: 157,
  TIRHUTA: 158,
  CAUCASIAN_ALBANIAN: 159,
  MAHAJANI: 160,
  AHOM: 161,
  HATRAN: 162,
  MODI: 163,
  MULTANI: 164,
  PAU_CIN_HAU: 165,
  SIDDHAM: 166,
};

/** @const @type {!Uint8Array} */
Unicode.CATEGORY_DATA;

/** @const @type {!Uint8Array} */
Unicode.SCRIPT_DATA;


/** @const @type {!repl.Console} */
var console;

/** @type {!Map<string, *>} */
TextDocument.prototype.properties;

/** @const @type {!Object} */
var Os;
