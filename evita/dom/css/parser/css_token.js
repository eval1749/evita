// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('css.Token');

goog.require('base');

goog.scope(function() {

const asStringLiteral = base.asStringLiteral;

class Token {
  /**
   * @param {!Token.Type} type
   * @param {string} source
   * @param {number} start
   * @param {number} end
   */
  constructor(type, source, start, end) {
    /** const @type {number} */
    this.end_ = end;
    /** const @type {source} */
    this.source_ = source;
    /** const @type {number} */
    this.start_ = start;
    /** const @type {number} */
    this.type_ = type;
  }

  /** @return {!Token.Type} */
  get type() { return this.type_; }

  /** @return {string} */
  get text() {
    if (this.type_ === Token.Type.END)
      return '';
    return this.source_.substr(this.start_, this.end_ - this.start_);
  }

  /**
   * @override
   * @return {string}
   */
  toString() {
    /** @const @type {string} */
    const source = asStringLiteral(this.text);
    return `Token(${this.typeString()}: ${source})`;
  }

  /** @return {string} */
  typeString() {
    for (let name in Token.Type) {
      if (Token.Type[name] === this.type_)
        return name;
    }
    return '?';
  }
}

/** @enum {number} */
Token.Type = {
  ASTERISK: 0,
  AT: 1,
  BAR: 2,
  COLON: 3,
  COMMA: 4,
  DELIM: 5,
  DIMENSION: 6,
  END: 7,
  FUNCTION: 8,
  HASH: 9,
  IDENT: 10,
  INVALID: 11,
  LBRACE: 12,
  LBRACKET: 13,
  LPAREN: 14,
  NUMBER: 15,
  OTHER: 16,
  PERCENT: 17,
  RBRACE: 18,
  RBRACKET: 19,
  RPAREN: 20,
  SEMICOLON: 21,
  STRING: 22,
  TILDE: 23,
  URL: 24,
  DOT: 25,
  DCOLON: 26,
};

/** @constructor */
css.Token = Token;
});
