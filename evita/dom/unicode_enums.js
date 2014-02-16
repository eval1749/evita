// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  // Well known character codes
  /** @const @type{number} */ Unicode.TAB = 0x09;
  /** @const @type{number} */ Unicode.LF = 0x0A;
  /** @const @type{number} */ Unicode.CR = 0x0D;
  /** @const @type{number} */ Unicode.SPACE = 0x20;
  /** @const @type{number} */ Unicode.EXCLAMATION_MARK = 0x21;
  /** @const @type{number} */ Unicode.QUOTATION_MARK = 0x22;
  /** @const @type{number} */ Unicode.NUMBER_SIGN = 0x23;
  /** @const @type{number} */ Unicode.DOLLAR_SIGN = 0x24;
  /** @const @type{number} */ Unicode.PERCENT_SIGN = 0x25;
  /** @const @type{number} */ Unicode.AMPERSAND = 0x26;
  /** @const @type{number} */ Unicode.APOSTROPHE = 0x27;
  /** @const @type{number} */ Unicode.LEFT_PARENTHESIS = 0x28;
  /** @const @type{number} */ Unicode.RIGHT_PARENTHESIS = 0x29;
  /** @const @type{number} */ Unicode.ASTERISK = 0x2A;
  /** @const @type{number} */ Unicode.PLUS_SIGN = 0x2B;
  /** @const @type{number} */ Unicode.COMMA = 0x2C;
  /** @const @type{number} */ Unicode.HYPHEN_MINUS = 0x2D;
  /** @const @type{number} */ Unicode.FULL_STOP = 0x2E;
  /** @const @type{number} */ Unicode.DIGIT_ZERO = 0x30;
  /** @const @type{number} */ Unicode.DIGIT_ONE = 0x31;
  /** @const @type{number} */ Unicode.DIGIT_TWO = 0x32;
  /** @const @type{number} */ Unicode.DIGIT_THREE = 0x33;
  /** @const @type{number} */ Unicode.DIGIT_FOUR = 0x34;
  /** @const @type{number} */ Unicode.DIGIT_FIVE = 0x35;
  /** @const @type{number} */ Unicode.DIGIT_SIX = 0x36;
  /** @const @type{number} */ Unicode.DIGIT_SEVEN = 0x37;
  /** @const @type{number} */ Unicode.DIGIT_EIGHT = 0x38;
  /** @const @type{number} */ Unicode.DIGIT_NINE = 0x39;
  /** @const @type{number} */ Unicode.COLON = 0x3A;
  /** @const @type{number} */ Unicode.SEMICOLON = 0x3B;
  /** @const @type{number} */ Unicode.LESS_THAN_SIGN = 0x3C;
  /** @const @type{number} */ Unicode.EQUALS_SIGN = 0x3D;
  /** @const @type{number} */ Unicode.GREATER_THAN_SIGN = 0x3E;
  /** @const @type{number} */ Unicode.QUESTION_MARK = 0x3F;
  /** @const @type{number} */ Unicode.COMMERCIAL_AT = 0x40;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_A = 0x41;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_B = 0x42;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_C = 0x43;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_D = 0x44;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_E = 0x45;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_F = 0x46;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_G = 0x47;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_H = 0x48;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_I = 0x49;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_J = 0x4A;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_K = 0x4B;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_L = 0x4C;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_M = 0x4D;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_N = 0x4E;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_O = 0x4F;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_P = 0x50;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_Q = 0x51;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_R = 0x52;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_S = 0x53;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_T = 0x54;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_U = 0x55;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_V = 0x56;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_W = 0x57;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_X = 0x58;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_Y = 0x59;
  /** @const @type{number} */ Unicode.LATIN_CAPITAL_LETTER_Z = 0x5A;
  /** @const @type{number} */ Unicode.LEFT_SQUARE_BRACKET = 0x5B;
  /** @const @type{number} */ Unicode.REVERSE_SOLIDUS = 0x5C;
  /** @const @type{number} */ Unicode.RIGHT_SQUARE_BRACKET = 0x5D;
  /** @const @type{number} */ Unicode.CIRCUMFLEX_ACCENT = 0x5E;
  /** @const @type{number} */ Unicode.LOW_LINE = 0x5F;
  /** @const @type{number} */ Unicode.GRAVE_ACCENT = 0x60;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_A = 0x61;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_B = 0x62;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_C = 0x63;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_D = 0x64;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_E = 0x65;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_F = 0x66;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_G = 0x67;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_H = 0x68;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_I = 0x69;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_J = 0x6A;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_K = 0x6B;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_L = 0x6C;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_M = 0x6D;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_N = 0x6E;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_O = 0x6F;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_P = 0x70;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_Q = 0x71;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_R = 0x72;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_S = 0x73;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_T = 0x74;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_U = 0x75;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_V = 0x76;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_W = 0x77;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_X = 0x78;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_Y = 0x79;
  /** @const @type{number} */ Unicode.LATIN_SMALL_LETTER_Z = 0x7A;
  /** @const @type{number} */ Unicode.LEFT_CURLY_BRACKET = 0x7B;
  /** @const @type{number} */ Unicode.VERTICAL_LINE = 0x7C;
  /** @const @type{number} */ Unicode.RIGHT_CURLY_BRACKET = 0x7D;
  /** @const @type{number} */ Unicode.TILDE = 0x7E;
  /** @const @type{number} */ Unicode.DELETE = 0x7F;

  /** @enum{string} */
  Unicode.CATEGORY_MAP = {
    Lu: 'Uppercase_Letter',
    Ll: 'Lowercase_Letter',
    Lt: 'Titlecase_Letter',
    LC: 'Cased_Letter',
    Lm: 'Modifier_Letter',
    Lo: 'Other_Letter',
    L: 'Letter',
    Mn: 'Nonspacing_Mark',
    Mc: 'Spacing_Mark',
    Me: 'Enclosing_Mark',
    M: 'Mark',
    Nd: 'Decimal_Number',
    Nl: 'Letter_Number',
    No: 'Other_Number',
    N: 'Number',
    Pc: 'Connector_Punctuation',
    Pd: 'Dash_Punctuation',
    Ps: 'Open_Punctuation',
    Pe: 'Close_Punctuation',
    Pi: 'Initial_Punctuation',
    Pf: 'Final_Punctuation',
    Po: 'Other_Punctuation',
    P: 'Punctuation',
    Sm: 'Math_Symbol',
    Sc: 'Currency_Symbol',
    So: 'Other_Symbol',
    S: 'Symbol',
    Zs: 'Space_Separator',
    Zl: 'Line_Separator',
    Zp: 'Paragraph_Separator',
    Z: 'Separator',
    Cc: 'Control',
    Cf: 'Format',
    Cs: 'Surrogate',
    Co: 'Private_Use',
    Cn: 'Unassigned',
    C: 'Other'
  };
})();

