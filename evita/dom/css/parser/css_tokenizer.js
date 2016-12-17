// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('css.Tokenizer');

goog.require('base.Logger');
goog.require('css.Token');
goog.require('unicode');

goog.scope(function() {

// Punctuation mapping:
//  '!'     other
//  '"'     string
//  '#'     hash or delim
//  '$'     delim
//  '%'     other
//  '&'     other
//  '\''    string
//  '('     lparen
//  ')'     rparen
//  '*'     delim
//  '+'     number or delim
//  ','     comma
//  '-'     number or delim
//  '.'     number or delim
//  '/'     other
//  ':'     colon
//  ';'     semicolon
//  '<'     delim
//  '='     other
//  '>'     other
//  '?'     other
//  '@'     at-keyword delim
//  '`'     other
//  '['     left-bracket
//  '\\'    escape or delm
//  ']'     right-bracket
//  '_'     other
//  '{'     left-brace
//  '|'     bar
//  '}'     right-brace
//  '~'     tilde

/** @constructor */
const Token = css.Token;

/** @const @type {!Map<number, !Token.Type>} */
const kPunctuationMap = new Map();
kPunctuationMap.set(Unicode.ASTERISK, Token.Type.DELIM);
kPunctuationMap.set(Unicode.DOLLAR_SIGN, Token.Type.DELIM);
kPunctuationMap.set(Unicode.CIRCUMFLEX_ACCENT, Token.Type.DELIM);
kPunctuationMap.set(Unicode.COMMA, Token.Type.COMMA);
kPunctuationMap.set(Unicode.LEFT_CURLY_BRACKET, Token.Type.LBRACE);
kPunctuationMap.set(Unicode.LEFT_PARENTHESIS, Token.Type.LPAREN);
kPunctuationMap.set(Unicode.LEFT_SQUARE_BRACKET, Token.Type.LBRACKET);
kPunctuationMap.set(Unicode.LESS_THAN_SIGN, Token.Type.DELIM);
kPunctuationMap.set(Unicode.RIGHT_CURLY_BRACKET, Token.Type.RBRACE);
kPunctuationMap.set(Unicode.RIGHT_PARENTHESIS, Token.Type.RPAREN);
kPunctuationMap.set(Unicode.RIGHT_SQUARE_BRACKET, Token.Type.RBRACKET);
kPunctuationMap.set(Unicode.SEMICOLON, Token.Type.SEMICOLON);
kPunctuationMap.set(Unicode.VERTICAL_LINE, Token.Type.BAR);
kPunctuationMap.set(Unicode.TILDE, Token.Type.TILDE);

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isDigitChar(charCode) {
  return charCode >= Unicode.DIGIT_ZERO && charCode <= Unicode.DIGIT_NINE;
}

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isNameChar(charCode) {
  return isNameStartChar(charCode) || isDigitChar(charCode) ||
      charCode === Unicode.HYPHEN_MINUS;
}

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isNameStartChar(charCode) {
  if (charCode >= Unicode.LATIN_CAPITAL_LETTER_A &&
      charCode <= Unicode.LATIN_CAPITAL_LETTER_Z) {
    return true;
  }
  if (charCode >= Unicode.LATIN_SMALL_LETTER_A &&
      charCode <= Unicode.LATIN_SMALL_LETTER_Z) {
    return true;
  }
  return charCode === Unicode.LOW_LINE || charCode >= 0x80;
}

/**
 * @param {!Token.Type} type
 * @param {string} source
 * @param {number} start
 * @param {number=} end
 * @return {!Token}
 */
function newToken(type, source, start, end = start + 1) {
  return new Token(type, source, start, end);
}

// Implements generator |Token| from CSS source code.
// TODO(eval1749): Support escape sequence:
//  \\ [^\n0-9A-Fa-f]+ | \\ [0-9A-Fa-f]{1,6}\s?
class Tokenizer extends base.Logger {
  constructor() { super(); }

  /**
   * @public
   * @param {string} source
   * @return {!Generator<!Token>}
   */
  * tokensOf(source) {
    /** @type {string} */
    let state = 'start';
    /** @type {number} */
    let tokenStart = 0;
    /** @type {!Token.Type} at end of source code*/
    let tokenType = Token.Type.END;

    for (let offset = 0; offset < source.length; ++offset) {
      /** @const @type {number} */
      const charCode = source.charCodeAt(offset);
      this.log(0, offset, source.substr(offset, 1), state);
      tokenType = Token.Type.INVALID;
      switch (state) {
        case 'start':
          tokenStart = offset;
          if (charCode === Unicode.APOSTROPHE) {
            state = 'string1';
            continue;
          }
          if (charCode === Unicode.ASTERISK) {
            yield newToken(Token.Type.ASTERISK, source, offset);
            tokenType = Token.Type.END;
            continue;
          }
          if (charCode === Unicode.COLON) {
            tokenType = Token.Type.COLON;
            state = ':';
            continue;
          }
          if (charCode === Unicode.COMMERCIAL_AT) {
            tokenType = Token.Type.DELIM;
            state = '@';
            continue;
          }
          if (charCode === Unicode.FULL_STOP) {
            tokenType = Token.Type.DELIM;
            state = '.';
            continue;
          }
          if (charCode === Unicode.NUMBER_SIGN) {
            tokenType = Token.Type.DELIM;
            state = '#';
            continue;
          }
          if (charCode === Unicode.QUOTATION_MARK) {
            state = 'string2';
            continue;
          }
          if (charCode === Unicode.SOLIDUS) {
            tokenType = Token.Type.DELIM;
            state = '/';
            continue;
          }
          if (charCode <= Unicode.SPACE) {
            tokenType = Token.Type.END;
            continue;
          }
          if (charCode === Unicode.HYPHEN_MINUS) {
            tokenType = Token.Type.DELIM;
            state = '-';
            continue;
          }
          if (charCode === Unicode.PLUS_SIGN) {
            tokenType = Token.Type.DELIM;
            state = '+';
            continue;
          }
          if (isDigitChar(charCode)) {
            tokenType = Token.Type.NUMBER;
            state = 'digit';
            continue;
          }
          if (isNameStartChar(charCode)) {
            tokenType = Token.Type.IDENT;
            state = 'ident';
            continue;
          }
          if (kPunctuationMap.has(charCode)) {
            /** @const {!Token.Type} */
            const type = kPunctuationMap.get(charCode) || Token.Type.INVALID;
            yield newToken(type, source, offset);
            tokenType = Token.Type.END;
            continue;
          }
          tokenType = Token.Type.END;
          yield newToken(Token.Type.OTHER, source, offset);
          continue;
        case 'digit':
          if (isDigitChar(charCode)) {
            tokenType = Token.Type.NUMBER;
            continue;
          }
          if (charCode === Unicode.FULL_STOP) {
            state = 'digit.';
            continue;
          }
          if (charCode === Unicode.LATIN_CAPITAL_LETTER_E ||
              charCode === Unicode.LATIN_SMALL_LETTER_E) {
            state = 'digitE';
            continue;
          }
          --offset;
          state = 'number';
          continue;
        case 'digit.':
          if (isDigitChar(charCode)) {
            tokenType = Token.Type.NUMBER;
            state = 'digit.digit';
            continue;
          }
          if (charCode === Unicode.LATIN_CAPITAL_LETTER_E ||
              charCode === Unicode.LATIN_SMALL_LETTER_E) {
            state = 'digitE';
            continue;
          }
          --offset;
          yield newToken(Token.Type.NUMBER, source, tokenStart, offset);
          --offset;
          state = 'start';
          continue;
        case 'digit.digit':
          if (isDigitChar(charCode)) {
            tokenType = Token.Type.NUMBER;
            continue;
          }
          if (charCode === Unicode.LATIN_CAPITAL_LETTER_E ||
              charCode === Unicode.LATIN_SMALL_LETTER_E) {
            state = 'digitE';
            continue;
          }
          --offset;
          state = 'number';
          continue;
        case 'digitE':
          if (isDigitChar(charCode)) {
            tokenType = Token.Type.NUMBER;
            state = 'digitEdigit';
            continue;
          }
          if (charCode === Unicode.HYPHEN_MINUS ||
              charCode === Unicode.PLUS_SIGN) {
            state = 'digitE+';
            continue;
          }
          // digit+ 'E'
          yield newToken(Token.Type.DIMENSION, source, tokenStart, offset);
          state = 'start';
          continue;
        case 'digitE+':
          if (isDigitChar(charCode)) {
            tokenType = Token.Type.NUMBER;
            state = 'digitEdigit';
            continue;
          }
          yield newToken(Token.Type.NUMBER, source, tokenStart, offset);
          state = 'start';
          continue;
        case 'digitEdigit':
          if (isDigitChar(charCode)) {
            tokenType = Token.Type.NUMBER;
            continue;
          }
          --offset;
          state = 'number';
          continue;
        case 'dimension':
          if (isNameChar(charCode)) {
            tokenType = Token.Type.DIMENSION;
            continue;
          }
          yield newToken(Token.Type.DIMENSION, source, tokenStart, offset);
          state = 'start';
          --offset;
          continue;
        case '.':
          if (isDigitChar(charCode)) {
            state = 'digit.';
            continue;
          }
          yield newToken(Token.Type.DOT, source, tokenStart, offset);
          state = 'start';
          --offset;
          continue;
        case '#':
          if (isNameChar(charCode)) {
            tokenType = Token.Type.HASH;
            state = '#name';
            continue;
          }
          yield newToken(Token.Type.DELIM, source, tokenStart, offset);
          state = 'start';
          --offset;
          continue;
        case '#name':
          if (isNameChar(charCode)) {
            tokenType = Token.Type.HASH;
            continue;
          }
          yield newToken(Token.Type.HASH, source, tokenStart, offset);
          state = 'start';
          --offset;
          continue;
        case '+':
          if (isDigitChar(charCode)) {
            tokenType = Token.Type.NUMBER;
            state = 'digit';
            continue;
          }
          yield newToken(Token.Type.DELIM, source, tokenStart, offset);
          state = 'start';
          --offset;
          continue;
        case '-':
          if (isDigitChar(charCode)) {
            tokenType = Token.Type.NUMBER;
            state = 'digit';
            continue;
          }
          if (isNameChar(charCode)) {
            tokenType = Token.Type.IDENT;
            state = 'ident';
            continue;
          }
          yield newToken(Token.Type.INVALID, source, tokenStart, offset);
          state = 'start';
          --offset;
          continue;
        case '/':
          if (charCode === Unicode.ASTERISK) {
            state = 'comment';
            continue;
          }
          yield newToken(Token.Type.OTHER, source, tokenStart, offset);
          state = 'start';
          --offset;
          continue;
        case ':':
          if (charCode === Unicode.COLON) {
            yield newToken(Token.Type.DCOLON, source, tokenStart, offset + 1);
            tokenType = Token.Type.END;
            state = 'start';
            continue;
          }
          yield newToken(Token.Type.COLON, source, tokenStart, offset);
          state = 'start';
          --offset;
          continue;
        case '@':
          if (isNameChar(charCode)) {
            tokenType = Token.Type.AT;
            continue;
          }
          yield newToken(Token.Type.DELIM, source, tokenStart, offset);
          state = 'start';
          --offset;
          continue;
        case 'comment':
          if (charCode === Unicode.ASTERISK)
            state = 'comment*';
          continue;
        case 'comment*':
          if (charCode === Unicode.ASTERISK)
            continue;
          if (charCode === Unicode.SOLIDUS) {
            tokenType = Token.Type.END;
            state = 'start';
            continue;
          }
          state = 'comment';
          continue;
        case 'ident':
          if (isNameChar(charCode)) {
            tokenType = Token.Type.IDENT;
            continue;
          }
          if (charCode === Unicode.LEFT_PARENTHESIS) {
            yield newToken(Token.Type.FUNCTION, source, tokenStart, offset + 1);
            tokenType = Token.Type.END;
            state = 'start';
            continue;
          }
          yield newToken(Token.Type.IDENT, source, tokenStart, offset);
          state = 'start';
          --offset;
          continue;
        case 'number':
          if (charCode === Unicode.PERCENT_SIGN) {
            tokenType = Token.Type.END;
            state = 'start';
            yield newToken(Token.Type.PERCENT, source, tokenStart, offset + 1);
            continue;
          }
          if (isNameStartChar(charCode)) {
            tokenType = Token.Type.DIMENSION;
            state = 'dimension';
            continue;
          }
          yield newToken(Token.Type.NUMBER, source, tokenStart, offset);
          state = 'start';
          --offset;
          continue;
        case 'string1':
          if (charCode !== Unicode.APOSTROPHE)
            continue;
          yield newToken(Token.Type.STRING, source, tokenStart + 1, offset + 1);
          tokenType = Token.Type.END;
          state = 'start';
          continue;
        case 'string2':
          if (charCode !== Unicode.QUOTATION_MARK)
            continue;
          yield newToken(Token.Type.STRING, source, tokenStart + 1, offset + 1);
          tokenType = Token.Type.END;
          state = 'start';
          continue;
        default:
          throw new Error(`Invalid state '${state}'`);
      }
    }

    yield newToken(tokenType, source, tokenStart, source.length);
    if (tokenType === Token.Type.END)
      return;
    yield newToken(Token.Type.END, source, source.length, source.length);
  }
}

/** @constructor */
css.Tokenizer = Tokenizer;
});
