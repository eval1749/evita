// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.PythonLexer = (function(keywords) {
  /** @const @type {!Map.<number, number>} */
  var CHARACTERS = (function() {
    var map = new Map();

    function setRange(type, min, max) {
      for (var charCode = min; charCode <= max; ++charCode) {
        map.set(charCode, type);
      }
    }

    map.set(Unicode.LF, Lexer.WHITESPACE_CHAR);
    map.set(Unicode.SPACE, Lexer.WHITESPACE_CHAR);
    map.set(Unicode.TAB, Lexer.WHITESPACE_CHAR);

    setRange(Lexer.OPERATOR_CHAR, 0x21, 0x3F);
    map.set(Unicode.FULL_STOP, Lexer.DOT_CHAR);

    // String literal
    map.set(Unicode.APOSTROPHE, Lexer.STRING1_CHAR);
    map.set(Unicode.QUOTATION_MARK, Lexer.STRING2_CHAR);

    // NameStartChar ::= [a-zA-Z_]
    setRange(Lexer.NAMESTART_CHAR,
             Unicode.LATIN_CAPITAL_LETTER_A,
             Unicode.LATIN_CAPITAL_LETTER_Z);
    setRange(Lexer.NAMESTART_CHAR,
             Unicode.LATIN_SMALL_LETTER_A,
             Unicode.LATIN_SMALL_LETTER_Z);
    map.set(Unicode.LOW_LINE, Lexer.NAMESTART_CHAR);

    // NameChar ::= NameStart | [0-9]
    setRange(Lexer.NAME_CHAR, Unicode.DIGIT_ZERO, Unicode.DIGIT_NINE);

    return map;
  })();

  /**
   * @constructor
   * @extends {Lexer}
   * @param {!Document} document
   */
  function PythonLexer(document) {
    Lexer.call(this, document, {
      characters: CHARACTERS,
      keywords: keywords
    });
  }

  /**
   * @this {!PythonLexer}
   * @param {!Lexer.Token} token
   * @return {!Lexer.State}
   */
  function didShrinkLastToken(token) {
    if (this.debug_ > 1)
      console.log('didShrinkLastToken', token);
    if (token.state == Lexer.State.DOT) {
      return Lexer.State.ZERO;
    }
    return token.state;
  }

  /**
   * @this {!ConfigLexer}
   * @param {number} charCode
   */
  function feedCharacter(charCode) {
    if (this.state == Lexer.State.ZERO) {
      switch (charCode) {
        case Unicode.FULL_STOP:
          this.startToken(Lexer.State.DOT);
          this.endToken();
          return;
        case Unicode.NUMBER_SIGN:
          this.startToken(Lexer.State.LINE_COMMENT);
          return;
      }
    }
    this.updateState(charCode);
  }

  /**
   * @this {!PythonLexer}
   * @param {!Lexer.Token} token
   * @param {!Range} range
   * @return {string}
   */
  function syntaxOfToken(range, token) {
    if (token.state != Lexer.State.WORD)
      return Lexer.prototype.syntaxOfToken.call(this, range, token);
    var lexer = this;
    var word = range.text;
    var it = lexer.tokens.find(token);
    console.assert(it, token);
    do {
      it = it.previous();
    } while (it && it.data.state == Lexer.State.SPACE);

    if (it && it.data.state == Lexer.State.DOT) {
      var tokens = lexer.collectTokens(it, token);
      return lexer.syntaxOfTokens(range, tokens);
    }

    return lexer.syntaxOfWord(word);
  }

  PythonLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: PythonLexer},
    didShrinkLastToken: {value: didShrinkLastToken },
    feedCharacter: {value: feedCharacter},
    syntaxOfToken: {value: syntaxOfToken}
  });

  // TODO(eval1749) Once closure compiler support |static get|, we should use
  // it.
  Object.defineProperty(PythonLexer, 'keywords', {
    get: function() { return keywords; }
  });
  return PythonLexer;
})(Lexer.createKeywords([
  // Keywords
  'and', 'as', 'assert', 'break', 'class',
  'continue', 'def', 'del', 'elif', 'else', 'except', 'exec',
  'finally', 'for', 'from', 'global', 'if', 'import', 'in', 'is',
  'lambda', 'not', 'or', 'pass', 'print', 'raise', 'return', 'try',
  'while', 'with', 'yield',
  // Reserved classes of identifiers
  '_*', '__*__', '__*', '__builtin__',
  // Built-in Constants
  'Ellipsis', 'False', 'None', 'NotImplemented', 'True', '__debug__',
  // Built-in Types
  'complex', 'float', 'int', 'long',
  'str', 'unicode', 'list', 'tuple', 'bytearray', 'xrange',
  'set', 'frozenset', 'dict',
  // 5.13. Special Attributes
  '__dict__', '__class__', '__bases__', '__name__', '__mro__',
  '__subclasses__',
]));
