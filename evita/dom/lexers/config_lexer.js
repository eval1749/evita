// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.ConfigLexer = (function() {
  /** @enum{number} */
  var State = {
    ZERO: 0, // State.ZERO must be zero.

    LINE_COMMENT: 300,
    LINE_COMMENT_START: 301,
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
  stateToSyntax.set(State.LINE_COMMENT, 'comment');
  stateToSyntax.set(State.LINE_COMMENT_START, 'comment');
  stateToSyntax.set(State.SPACE, '');
  stateToSyntax.set(State.STRING1, 'string_literal');
  stateToSyntax.set(State.STRING1_END, 'string_literal');
  stateToSyntax.set(State.STRING1_ESCAPE, 'string_literal');
  stateToSyntax.set(State.STRING2, 'string_literal');
  stateToSyntax.set(State.STRING2_END, 'string_literal');
  stateToSyntax.set(State.STRING2_ESCAPE, 'string_literal');
  stateToSyntax.set(State.WORD, 'normal');

  Object.keys(State).forEach(function(key) {
    if (!stateToSyntax.has(State[key]))
      throw new Error('stateToSyntax must have ' + key);
  });

  /**
   * @constructor
   * @extends Lexer
   * @param {!Document} document
   */
  function ConfigLexer(document) {
    Lexer.call(this, document, {
      keywords: [],
      stateToSyntax: stateToSyntax
    });
  }

  /**
   * @param {number} charCode
   * @return {boolean}
   */
  function isWhitespace(charCode) {
    return charCode == Unicode.LF || charCode == Unicode.SPACE ||
           charCode == Unicode.TAB;
  }

  /**
   * @param {number} charCode
   * @return {boolean}
   */
  function isWordRest(charCode) {
    return !isWhitespace(charCode) &&
           charCode != Unicode.APOSTROPHE &&
           charCode != Unicode.NUMBER_SIGN &&
           charCode != Unicode.QUOTATION_MARK;
  }

  /**
   * @this {!ConfigLexer}
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

        case State.LINE_COMMENT_START:
          if (charCode == Unicode.LF) {
            lexer.state = State.ZERO;
            break;
          }
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
          if (!isWordRest(charCode))
            return lexer.finishToken(State.ZERO);
          lexer.extendToken();
          break;

        case State.ZERO:
          switch (charCode) {
            case Unicode.APOSTROPHE:
              lexer.startToken(State.STRING1);
              break;
            case Unicode.NUMBER_SIGN:
              lexer.startToken(State.LINE_COMMENT_START);
              return lexer.finishToken(State.LINE_COMMENT);
            case Unicode.QUOTATION_MARK:
              lexer.startToken(State.STRING2);
              break;
            case Unicode.SPACE:
            case Unicode.TAB:
              lexer.startToken(State.SPACE);
              break;
            default:
              lexer.startToken(State.WORD);
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

  ConfigLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: ConfigLexer},
    nextToken: {value: nextToken}
  });

  return ConfigLexer;
})();
