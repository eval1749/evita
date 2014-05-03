// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.ClikeLexer = (function() {
  /** @enum{number} */
  var State = {
    ZERO: 0, // State.ZERO must be zero.

    BLOCK_COMMENT: 100,
    BLOCK_COMMENT_ASTERISK: 101,
    BLOCK_COMMENT_END: 102,
    BLOCK_COMMENT_START: 103,
    COLON: 200,
    COLON_COLON: 201,
    DOT: 202,
    LINE_COMMENT: 300,
    LINE_COMMENT_ESCAPE: 301,
    LINE_COMMENT_START: 302,
    NEWLINE: 400,
    NUMBER_SIGN: 401,
    OPERATOR: 500,
    SOLIDUS: 600,
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
  stateToSyntax.set(State.BLOCK_COMMENT, 'comment');
  stateToSyntax.set(State.BLOCK_COMMENT_ASTERISK, 'comment');
  stateToSyntax.set(State.BLOCK_COMMENT_END, 'comment');
  stateToSyntax.set(State.BLOCK_COMMENT_START, 'comment');
  stateToSyntax.set(State.COLON, 'operators');
  stateToSyntax.set(State.COLON_COLON, 'operators');
  stateToSyntax.set(State.DOT, 'operators');
  stateToSyntax.set(State.LINE_COMMENT, 'comment');
  stateToSyntax.set(State.LINE_COMMENT_ESCAPE, 'comment');
  stateToSyntax.set(State.LINE_COMMENT_START, 'comment');
  stateToSyntax.set(State.NEWLINE, '');
  stateToSyntax.set(State.NUMBER_SIGN, 'operators');
  stateToSyntax.set(State.OPERATOR, 'operators');
  stateToSyntax.set(State.SOLIDUS, 'operators');
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
    attrs.set(Unicode.SOLIDUS, ClikeLexer.SLASH_CHAR);

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
   * @param {!ClikeLexerOptions} options
   */
  function ClikeLexer(document, options) {
    Lexer.call(this, document, {
      characters: options.characters,
      keywords: options.keywords,
      stateToSyntax: stateToSyntax
    });
    this.hasCpp = options.hasCpp;
    this.useColonColon = options.useColonColon;
    this.useDot = options.useDot;
  }

  /**
   * @this {!ClikeLexer}
   * @param {!Lexer.Token} token
   */
  function didShrinkLastToken(token) {
    if (this.debug_ > 1)
      console.log('didShrinkLastToken', token);
    if (token.state == State.BLOCK_COMMENT_END) {
      token.state = State.BLOCK_COMMENT_ASTERISK;
      return;
    }
    if (token.state == State.BLOCK_COMMENT_START) {
      token.state = State.SOLIDUS;
      return;
    }
    if (token.state == State.COLON_COLON) {
      token.state = State.COLON;
      return;
    }
    if (token.state == State.DOT) {
      token.state = State.ZERO;
      return;
    }
    if (token.state == State.LINE_COMMENT_START) {
      token.state = State.SOLIDUS;
      return;
    }
  }

  /**
   * @this {!ClikeLexer}
   * @param {!Range} range
   * @param {!Lexer.Token} token
   * @return {string}
   *
   * Extract CPP directive as '#" + word or namespace qualify identifier.
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

    if (it.data.state == State.NUMBER_SIGN) {
      range.start = it.data.start;
      word = '#' + word;
      return word;
    }

    while (it && it.data.state == State.COLON_COLON) {
      range.start = it.data.start;
      word = '::' + word;
      do {
        it = it.previous();
        if (!it)
          return word;
      } while (it.data.state == State.SPACE);
      if (it.data.state != State.WORD)
        return word;
      var namespace = it.data;
      range.start = namespace.start;
      word = range.document.slice(namespace.start, namespace.end) + word;
      it = it.previous();
    }
    return word;
  }

  /**
   * @this {!ClikeLexer}
   * @param {number} maxOffset
   * @return {?Lexer.Token}
   */
  function nextToken(maxOffset) {
    var lexer = this;
    var document = lexer.range.document;
    while (lexer.scanOffset < maxOffset) {
      var charCode = document.charCodeAt_(lexer.scanOffset);
      switch (lexer.state) {
        case State.BLOCK_COMMENT:
          if (charCode == Unicode.ASTERISK)
            return lexer.finishToken(State.BLOCK_COMMENT_ASTERISK);
          lexer.extendToken();
          break;

        case State.BLOCK_COMMENT_ASTERISK:
          if (charCode == Unicode.ASTERISK) {
            lexer.extendToken();
            break;
          }
          if (charCode == Unicode.SOLIDUS)
            return lexer.finishTokenAs(State.BLOCK_COMMENT_END);
          return lexer.finishToken(State.BLOCK_COMMENT);

        case State.BLOCK_COMMENT_END:
          lexer.state = State.ZERO;
          break;

        case State.BLOCK_COMMENT_START:
          if (charCode == Unicode.ASTERISK)
            lexer.startToken(State.BLOCK_COMMENT_ASTERISK);
          else
            lexer.startToken(State.BLOCK_COMMENT);
          break;

        case State.COLON:
          if (charCode == Unicode.COLON)
            return lexer.finishTokenAs(State.COLON_COLON);
          return lexer.finishToken(State.ZERO);

        case State.COLON_COLON:
        case State.DOT:
          lexer.state = State.ZERO;
          break;

        case State.LINE_COMMENT:
          if (charCode == Unicode.LF)
            return lexer.finishToken(State.ZERO);
          if (charCode == Unicode.REVERSE_SOLIDUS)
            return lexer.finishToken(State.LINE_COMMENT_ESCAPE);
          lexer.extendToken();
          break;

        case State.LINE_COMMENT_ESCAPE:
          return lexer.finishToken(State.LINE_COMMENT);

        case State.LINE_COMMENT_START:
          if (charCode == Unicode.LF) {
            lexer.state = State.ZERO;
            break;
          }
          if (charCode == Unicode.REVERSE_SOLIDUS)
            lexer.startToken(State.LINE_COMMENT_ESCAPE);
          else
            lexer.startToken(State.LINE_COMMENT);
          break;

        case State.NEWLINE:
          if (!lexer.isWhitespace(charCode))
            return lexer.finishToken(State.ZERO);
          lexer.extendToken();
          break;

        case State.OPERATOR:
          if (!lexer.isOperator(charCode))
            return lexer.finishToken(State.ZERO);
          lexer.extendToken();
          break;

        case State.SOLIDUS:
          if (charCode == Unicode.ASTERISK)
            return lexer.finishTokenAs(State.BLOCK_COMMENT_START);
          if (charCode == Unicode.SOLIDUS)
            return lexer.finishTokenAs(State.LINE_COMMENT_START);
          return lexer.finishToken(State.ZERO);

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
            case Unicode.LF:
              lexer.startToken(State.NEWLINE);
              break;
            case Unicode.QUOTATION_MARK:
              lexer.startToken(State.STRING2);
              break;
            case Unicode.SOLIDUS:
              lexer.startToken(State.SOLIDUS);
              break;
            case Unicode.SPACE:
            case Unicode.TAB:
              lexer.startToken(State.SPACE);
              break;
            default:
              if (lexer.hasCpp && charCode == Unicode.NUMBER_SIGN) {
                if (!lexer.lastToken ||
                    lexer.lastToken.state == State.NEWLINE) {
                  lexer.startToken(State.NUMBER_SIGN);
                  return lexer.finishToken(State.ZERO);
                }
                lexer.startToken(State.OPERATOR);
                break;
              }

              if (lexer.useColonColon && charCode == Unicode.COLON) {
                lexer.startToken(State.COLON);
                break;
              }

              if (lexer.useDot && charCode == Unicode.FULL_STOP) {
                lexer.startToken(State.DOT);
                return lexer.finishToken(State.ZERO);
              }

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

  ClikeLexer.newCharacters = function() {
    var attrs = new Map();
    CHARACTERS.forEach(function(value, key) {
      attrs.set(key, value);
    });
    return attrs;
  };

  ClikeLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: ClikeLexer},
    didShrinkLastToken: {value: didShrinkLastToken },
    extractWord: {value: extractWord},
    nextToken: {value: nextToken}
  });

  return ClikeLexer;
})();
