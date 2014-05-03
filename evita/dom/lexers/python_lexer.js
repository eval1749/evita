// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.PythonLexer = (function(keywords) {
  /** @enum{number} */
  var State = {
    ZERO: 0, // State.ZERO must be zero.

    DOT: 202,
    LINE_COMMENT: 300,
    OPERATOR: 500,
    SPACE: 700,
    STRING1: 800,
    STRING1_END: 801,
    STRING1_ESCAPE: 802,
    STRING2: 810,
    STRING2_END: 811,
    STRING2_ESCAPE: 812,
    WORD: 900
  };

  /** @const @type {!Map.<State, string>} */
  var stateToSyntax = new Map();
  stateToSyntax.set(State.ZERO, '');
  stateToSyntax.set(State.DOT, 'operators');
  stateToSyntax.set(State.LINE_COMMENT, 'comment');
  stateToSyntax.set(State.OPERATOR, 'operators');
  stateToSyntax.set(State.SPACE, '');
  stateToSyntax.set(State.STRING1, 'string_literal');
  stateToSyntax.set(State.STRING1_END, 'string_literal');
  stateToSyntax.set(State.STRING1_ESCAPE, 'string_literal');
  stateToSyntax.set(State.STRING2, 'string_literal');
  stateToSyntax.set(State.STRING2_END, 'string_literal');
  stateToSyntax.set(State.STRING2_ESCAPE, 'string_literal');
  stateToSyntax.set(State.WORD, 'identifier');

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
   * @extends Lexer
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
      token.state = State.ZERO;
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
    } while (it && it.data.state == State.SPACE);

    if (!it)
      return word;

    if (it.data.state == State.DOT) {
      word = '.' + word;
      if (lexer.keywords.has(word)) {
        range.start = it.data.start;
        return word;
      }
      it = it.previous();
      while (it && it.data.state == State.SPACE) {
        it = it.previous();
      }
      if (!it || it.data.state != State.WORD)
        return word;
      var previous = range.document.slice(it.data.start, it.data.end);
      return previous + word;
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
      switch (lexer.state) {
        case State.LINE_COMMENT:
          if (charCode == Unicode.LF)
            return lexer.finishToken(State.ZERO);
          lexer.extendToken();
          break;

        case State.OPERATOR:
          if (!lexer.isOperator(charCode))
            return lexer.finishToken(State.ZERO);
          lexer.extendToken();
          break;

        case State.SPACE:
          if (charCode != Unicode.SPACE && charCode != Unicode.TAB)
            return lexer.finishToken(State.ZERO);
          lexer.extendToken();
          break;

        case State.STRING1:
          if (charCode == Unicode.APOSTROPHE)
            return lexer.finishToken(State.STRING2_END);
          if (charCode == Unicode.REVERSE_SOLIDUS)
            return lexer.finishToken(State.STRING1_ESCAPE);
          lexer.extendToken();
          break;
        case State.STRING1_END:
          return lexer.finishToken(State.ZERO);
        case State.STRING1_ESCAPE:
          return lexer.finishToken(State.STRING1);

        case State.STRING2:
          if (charCode == Unicode.QUOTATION_MARK)
            return lexer.finishToken(State.STRING2_END);
          if (charCode == Unicode.REVERSE_SOLIDUS)
            return lexer.finishToken(State.STRING2_ESCAPE);
          lexer.extendToken();
          break;
        case State.STRING2_END:
          return lexer.finishToken(State.ZERO);
        case State.STRING2_ESCAPE:
          return lexer.finishToken(State.STRING2);

        case State.WORD:
          if (!lexer.isWord(charCode))
            return lexer.finishToken(State.ZERO);
          lexer.extendToken();
          break;

        case State.ZERO:
          switch (charCode) {
            case Unicode.APOSTROPHE:
              lexer.startToken(State.STRING1);
              break;
            case Unicode.FULL_STOP:
              lexer.startToken(State.DOT);
              return lexer.finishToken(State.ZERO);
            case Unicode.LF:
            case Unicode.SPACE:
            case Unicode.TAB:
              lexer.startToken(State.SPACE);
              break;
            case Unicode.NUMBER_SIGN:
              lexer.startToken(State.LINE_COMMENT);
              break;
            case Unicode.QUOTATION_MARK:
              lexer.startToken(State.STRING2);
              break;
            default:
              if (lexer.isWord(charCode))
                lexer.startToken(State.WORD);
              else
                lexer.startToken(State.OPERATOR);
              break;
          }
          break;
        default:
          console.log(lexer);
          throw new Error('Invalid state ' + lexer.state);
      }
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
