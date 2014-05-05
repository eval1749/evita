// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.PythonLexer = (function(keywords) {
  /** @const @type {!Map.<number, number>} */
  var CHARACTERS = (function() {
    var map = new Map();

    map.set(Unicode.LF, Lexer.WHITESPACE_CHAR);
    map.set(Unicode.SPACE, Lexer.WHITESPACE_CHAR);
    map.set(Unicode.TAB, Lexer.WHITESPACE_CHAR);

    for (var charCode = 0x21; charCode < 0x40; ++charCode) {
      map.set(charCode, Lexer.OPERATOR_CHAR);
    }

    map.set(Unicode.FULL_STOP, Lexer.DOT_CHAR);

    // String literal
    map.set(Unicode.APOSTROPHE, Lexer.STRING1_CHAR);
    map.set(Unicode.QUOTATION_MARK, Lexer.STRING2_CHAR);

    // Word [0-9a-zA-Z_]
    for (var charCode = Unicode.DIGIT_ZERO;
         charCode <= Unicode.DIGIT_NINE; ++charCode) {
      map.set(charCode, Lexer.WORD_CHAR);
    }
    for (var charCode = Unicode.LATIN_CAPITAL_LETTER_A;
         charCode <= Unicode.LATIN_CAPITAL_LETTER_Z; ++charCode) {
      map.set(charCode, Lexer.WORD_CHAR);
    }
    for (var charCode = Unicode.LATIN_SMALL_LETTER_A;
         charCode <= Unicode.LATIN_SMALL_LETTER_Z; ++charCode) {
      map.set(charCode, Lexer.WORD_CHAR);
    }
    map.set(Unicode.LOW_LINE, Lexer.WORD_CHAR);

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
   */
  function didShrinkLastToken(token) {
    if (this.debug_ > 1)
      console.log('didShrinkLastToken', token);
    if (token.state == Lexer.State.DOT) {
      token.state = Lexer.State.ZERO;
      return;
    }
  }

  /**
   * @this {!PythonLexer}
   * @param {number} maxOffset
   */
  function nextToken(maxOffset) {
    var lexer = this;
    var document = lexer.range.document;
    while (lexer.scanOffset < maxOffset) {
      var charCode = document.charCodeAt_(lexer.scanOffset);
      if (lexer.state == Lexer.State.ZERO) {
        switch (charCode) {
          case Unicode.FULL_STOP:
            lexer.startToken(Lexer.State.DOT);
            lexer.endToken();
            continue;
          case Unicode.NUMBER_SIGN:
            lexer.startToken(Lexer.State.LINE_COMMENT);
            continue;
        }
      }
      this.updateState(charCode);
    }
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
    nextToken: {value: nextToken},
    syntaxOfToken: {value: syntaxOfToken}
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
