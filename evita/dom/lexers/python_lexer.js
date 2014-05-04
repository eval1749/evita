// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.PythonLexer = (function(keywords) {
  /** @enum{!Symbol} */
  var State = {
    DOT: Symbol('.'),
  };

  /** @const @type {!Map.<State, string>} */
  var stateToSyntax = new Map();
  stateToSyntax.set(State.DOT, 'operators');

  Object.keys(State).forEach(function(key) {
    if (!stateToSyntax.has(State[key]))
      throw new Error('stateToSyntax must have ' + key);
  });

  /** @const @type {!Map.<number, number>} */
  var CHARACTERS = (function() {
    var attrs = new Map();

    attrs.set(Unicode.LF, Lexer.WHITESPACE_CHAR);
    attrs.set(Unicode.SPACE, Lexer.WHITESPACE_CHAR);
    attrs.set(Unicode.TAB, Lexer.WHITESPACE_CHAR);

    for (var charCode = 0x21; charCode < 0x40; ++charCode) {
      attrs.set(charCode, Lexer.OPERATOR_CHAR);
    }

    attrs.set(Unicode.FULL_STOP, ClikeLexer.DOT_CHAR);

    // String literal
    attrs.set(Unicode.APOSTROPHE, Lexer.STRING1_CHAR);
    attrs.set(Unicode.QUOTATION_MARK, Lexer.STRING2_CHAR);

    // Word [0-9a-zA-Z_]
    for (var charCode = Unicode.DIGIT_ZERO;
         charCode <= Unicode.DIGIT_NINE; ++charCode) {
      attrs.set(charCode, Lexer.WORD_CHAR);
    }
    for (var charCode = Unicode.LATIN_CAPITAL_LETTER_A;
         charCode <= Unicode.LATIN_CAPITAL_LETTER_Z; ++charCode) {
      attrs.set(charCode, Lexer.WORD_CHAR);
    }
    for (var charCode = Unicode.LATIN_SMALL_LETTER_A;
         charCode <= Unicode.LATIN_SMALL_LETTER_Z; ++charCode) {
      attrs.set(charCode, Lexer.WORD_CHAR);
    }
    attrs.set(Unicode.LOW_LINE, Lexer.WORD_CHAR);

    return attrs;
  })();

  /**
   * @constructor
   * @extends {Lexer}
   * @param {!Document} document
   */
  function PythonLexer(document) {
    Lexer.call(this, document, {
      characters: CHARACTERS,
      keywords: keywords,
      stateToSyntax: stateToSyntax
    });
  }

  /**
   * @this {!PythonLexer}
   * @param {!Lexer.Token} token
   */
  function didShrinkLastToken(token) {
    if (this.debug_ > 1)
      console.log('didShrinkLastToken', token);
    if (token.state == State.DOT) {
      token.state = Lexer.State.ZERO;
      return;
    }
  }

  /**
   * @this {!PythonLexer}
   * @param {!Range} range
   * @param {!Lexer.Token} token
   * @return {string}
   */
  function extractWord(range, token) {
    var lexer = this;
    var word = range.text;
    var it = lexer.tokens.find(token);
    console.assert(it, token);
    do {
      it = it.previous();
    } while (it && it.data.state == Lexer.State.SPACE);

    if (!it)
      return word;

    if (it.data.state == State.DOT) {
      var dotWord = '.' + word;
      if (lexer.keywords.has(dotWord)) {
        range.start = it.data.start;
        return dotWord;
      }
      it = it.previous();
      while (it && it.data.state == Lexer.State.SPACE) {
        it = it.previous();
      }
      if (!it || it.data.state != Lexer.State.WORD)
        return word;
      range.start = it.data.start;
      var previous = range.document.slice(it.data.start, it.data.end);
      return previous + dotWord;
    }

    return word;
  }

  /**
   * @this {!PythonLexer}
   * @param {number} maxOffset
   * @return {?Lexer.Token}
   */
  function nextToken(maxOffset) {
    var lexer = this;
    var document = lexer.range.document;
    while (lexer.scanOffset < maxOffset) {
      var charCode = document.charCodeAt_(lexer.scanOffset);
      if (lexer.state == Lexer.State.ZERO) {
        switch (charCode) {
          case Unicode.FULL_STOP:
            lexer.startToken(State.DOT);
            return lexer.finishToken(Lexer.State.ZERO);
          case Unicode.NUMBER_SIGN:
            lexer.startToken(Lexer.State.LINE_COMMENT);
            continue;
        }
      }
      var token = this.updateState(charCode);
      if (token)
        return token;
    }
    return lexer.lastToken;
  }

  PythonLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: PythonLexer},
    didShrinkLastToken: {value: didShrinkLastToken },
    extractWord: {value: extractWord},
    nextToken: {value: nextToken}
  });

  return PythonLexer;
})([
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
]);
