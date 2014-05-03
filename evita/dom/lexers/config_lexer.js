// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.ConfigLexer = (function() {
  /** @enum{number} */
  var State = {
    ZERO: 0, // State.ZERO must be zero.

    LINE_COMMENT: 300,
    LINE_COMMENT_START: 301,
    OTHER: 400,
    SPACE: 700,
    STRING1: 800,
    STRING1_END: 801,
    STRING1_ESCAPE: 802,
    STRING2: 810,
    STRING2_END: 811,
    STRING2_ESCAPE: 812,
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
  stateToSyntax.set(State.OTHER, '');

  Object.keys(State).forEach(function(key) {
    if (!stateToSyntax.has(State[key]))
      throw new Error('stateToSyntax must have ' + key);
  });

  /** @const @type {!Map.<number, !Symbol>} */
  var CHARACTERS = (function() {
    var attrs = new Map();
    attrs.set(Unicode.LF, Lexer.WHITESPACE_CHAR);
    attrs.set(Unicode.SPACE, Lexer.WHITESPACE_CHAR);
    attrs.set(Unicode.TAB, Lexer.WHITESPACE_CHAR);
    attrs.set(Unicode.APOSTROPHE, Lexer.STRING1_CHAR);
    attrs.set(Unicode.QUOTATION_MARK, Lexer.STRING2_CHAR);
    return attrs;
  })();

  /**
   * @constructor
   * @extends Lexer
   * @param {!Document} document
   */
  function ConfigLexer(document) {
    Lexer.call(this, document, {
      characters: CHARACTERS,
      keywords: [],
      stateToSyntax: stateToSyntax
    });
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

        case State.OTHER:
          if (!this.isOther(charCode))
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
            case Unicode.LF:
            case Unicode.SPACE:
            case Unicode.TAB:
              lexer.startToken(State.SPACE);
              break;
            default:
              console.assert(lexer.isOther(charCode));
              lexer.startToken(State.OTHER);
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
