// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.ClikeLexer = (function() {
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
    map.set(Unicode.SOLIDUS, ClikeLexer.SLASH_CHAR);

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

  /** @enum{!Symbol} */
  ClikeLexer.State = {
    BLOCK_COMMENT: Symbol('block_comment'),
    BLOCK_COMMENT_ASTERISK: Symbol('block_comment_asterisk'),
    BLOCK_COMMENT_END: Symbol('block_comment_end'),
    BLOCK_COMMENT_START: Symbol('block_comment_start'),
    COLON: Symbol('::'),
    COLON_COLON: Symbol('::'),
    LINE_COMMENT: Symbol('line_comment'),
    LINE_COMMENT_ESCAPE: Symbol('line_comment_escape'),
    LINE_COMMENT_START: Symbol('line_comment_end'),
    NEWLINE: Symbol('newline'),
    NUMBER_SIGN: Symbol('#'),
    SOLIDUS: Symbol('/'),
  };

  /** @const @type {!Map.<ClikeLexer.State, string>} */
  var STATE_TO_SYNTAX = (function() {
    var map = new Map();
    map.set(ClikeLexer.State.BLOCK_COMMENT, 'comment');
    map.set(ClikeLexer.State.BLOCK_COMMENT_ASTERISK, 'comment');
    map.set(ClikeLexer.State.BLOCK_COMMENT_END, 'comment');
    map.set(ClikeLexer.State.BLOCK_COMMENT_START, 'comment');
    map.set(ClikeLexer.State.COLON, 'operators');
    map.set(ClikeLexer.State.COLON_COLON, 'operators');
    map.set(ClikeLexer.State.LINE_COMMENT, 'comment');
    map.set(ClikeLexer.State.LINE_COMMENT_ESCAPE, 'comment');
    map.set(ClikeLexer.State.LINE_COMMENT_START, 'comment');
    map.set(ClikeLexer.State.NEWLINE, '');
    map.set(ClikeLexer.State.NUMBER_SIGN, 'operators');
    map.set(ClikeLexer.State.SOLIDUS, 'operators');

    Object.keys(ClikeLexer.State).forEach(function(key) {
      if (!map.has(ClikeLexer.State[key]))
        throw new Error('map must have ' + key);
    });
    return map;
  })();

  /**
   * @constructor
   * @extends {Lexer}
   * @param {!Document} document
   * @param {!ClikeLexerOptions} options
   */
  function ClikeLexer(document, options) {
    Lexer.call(this, document, {
      characters: options.characters,
      keywords: options.keywords
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
    switch (token.state) {
      case ClikeLexer.State.BLOCK_COMMENT_END:
        token.state = ClikeLexer.State.BLOCK_COMMENT_ASTERISK;
        return;
      case ClikeLexer.State.BLOCK_COMMENT_START:
        token.state = ClikeLexer.State.SOLIDUS;
        return;
      case ClikeLexer.State.COLON_COLON:
        token.state = ClikeLexer.State.COLON;
        return;
      case Lexer.State.DOT:
        token.state = Lexer.State.ZERO;
        return;
      case ClikeLexer.State.LINE_COMMENT_START:
        token.state = ClikeLexer.State.SOLIDUS;
        return;
    }
  }

  /**
   * @this {!ClikeLexer}
   * @param {number} maxOffset
   */
  function nextToken(maxOffset) {
    var lexer = this;
    var document = lexer.range.document;
    while (lexer.scanOffset < maxOffset) {
      var charCode = document.charCodeAt_(lexer.scanOffset);
      switch (lexer.state) {
        case ClikeLexer.State.BLOCK_COMMENT:
          if (charCode == Unicode.ASTERISK)
            lexer.finishToken(ClikeLexer.State.BLOCK_COMMENT_ASTERISK);
          else
            lexer.extendToken();
          continue;

        case ClikeLexer.State.BLOCK_COMMENT_ASTERISK:
          if (charCode == Unicode.ASTERISK)
            lexer.extendToken();
          else if (charCode == Unicode.SOLIDUS)
            lexer.restartToken(ClikeLexer.State.BLOCK_COMMENT_END);
          else
            lexer.finishToken(ClikeLexer.State.BLOCK_COMMENT);
          continue;

        case ClikeLexer.State.BLOCK_COMMENT_END:
          lexer.state = Lexer.State.ZERO;
          continue;

        case ClikeLexer.State.BLOCK_COMMENT_START:
          if (charCode == Unicode.ASTERISK)
            lexer.startToken(ClikeLexer.State.BLOCK_COMMENT_ASTERISK);
          else
            lexer.startToken(ClikeLexer.State.BLOCK_COMMENT);
          continue;

        case ClikeLexer.State.COLON:
          if (charCode == Unicode.COLON)
            lexer.restartToken(ClikeLexer.State.COLON_COLON);
          else
            lexer.endToken();
          continue;

        case ClikeLexer.State.COLON_COLON:
          lexer.state = Lexer.State.ZERO;
          continue;

        case ClikeLexer.State.LINE_COMMENT:
          if (charCode == Unicode.LF)
            lexer.endToken();
          else if (charCode == Unicode.REVERSE_SOLIDUS)
            lexer.finishToken(ClikeLexer.State.LINE_COMMENT_ESCAPE);
          else
            lexer.extendToken();
          continue;

        case ClikeLexer.State.LINE_COMMENT_ESCAPE:
          lexer.finishToken(ClikeLexer.State.LINE_COMMENT);
          continue;

        case ClikeLexer.State.LINE_COMMENT_START:
          if (charCode == Unicode.LF) {
            lexer.state = Lexer.State.ZERO;
            continue;
          }
          if (charCode == Unicode.REVERSE_SOLIDUS)
            lexer.startToken(ClikeLexer.State.LINE_COMMENT_ESCAPE);
          else
            lexer.startToken(ClikeLexer.State.LINE_COMMENT);
          continue;

        case ClikeLexer.State.NEWLINE:
          if (lexer.isWhitespace(charCode))
            lexer.extendToken();
          else
            lexer.endToken();
          continue;

        case ClikeLexer.State.NUMBER_SIGN:
          lexer.state = Lexer.State.ZERO;
          continue;

        case ClikeLexer.State.SOLIDUS:
          if (charCode == Unicode.ASTERISK)
            lexer.restartToken(ClikeLexer.State.BLOCK_COMMENT_START);
          else if (charCode == Unicode.SOLIDUS)
            lexer.restartToken(ClikeLexer.State.LINE_COMMENT_START);
          else
            lexer.endToken();
          continue;

        case Lexer.State.ZERO:
          switch (charCode) {
            case Unicode.LF:
              lexer.startToken(ClikeLexer.State.NEWLINE);
              continue;
            case Unicode.SOLIDUS:
              lexer.startToken(ClikeLexer.State.SOLIDUS);
              continue;
            default:
              if (lexer.hasCpp && charCode == Unicode.NUMBER_SIGN) {
                if (!lexer.lastToken ||
                    lexer.lastToken.state == ClikeLexer.State.NEWLINE) {
                  lexer.startToken(ClikeLexer.State.NUMBER_SIGN);
                  lexer.endToken();
                  continue;
                }
                lexer.startToken(Lexer.State.OPERATOR);
                continue;
              }

              if (lexer.useColonColon && charCode == Unicode.COLON) {
                lexer.startToken(ClikeLexer.State.COLON);
                continue;
              }

              if (lexer.useDot && charCode == Unicode.FULL_STOP) {
                lexer.startToken(Lexer.State.DOT);
                lexer.endToken();
                continue;
              }
              break;
          }
          break;
      }
      this.updateState(charCode);
    }
  }

  /**
   * @this {!CppLexer}
   * @param {!Range} range
   * @param {!Lexer.Token} token
   * @return {string}
   */
  function syntaxOfToken(range, token) {
    if (token.state != Lexer.State.WORD) {
      return STATE_TO_SYNTAX.get(token.state) ||
             Lexer.prototype.syntaxOfToken.call(this, range, token);
    }
    var lexer = this;
    var document = range.document;
    var word = range.text;
    var it = lexer.tokens.find(token);
    console.assert(it, token);
    do {
      it = it.previous();
    } while (it && it.data.state == Lexer.State.SPACE);

    if (!it)
      return lexer.syntaxOfWord(word);

    var delimiter = it.data;
    if (delimiter.state == ClikeLexer.State.NUMBER_SIGN) {
      return lexer.syntaxOfTokens(range, [delimiter, token]);
    }

    if (delimiter.state == ClikeLexer.State.COLON_COLON ||
        delimiter.state == Lexer.State.DOT) {
      var tokens = lexer.collectTokens(it, token);
      return lexer.syntaxOfTokens(range, tokens);
    }

    return lexer.syntaxOfWord(word);
  }

  ClikeLexer.newCharacters = function() {
    var map = new Map();
    CHARACTERS.forEach(function(value, key) {
      map.set(key, value);
    });
    return map;
  };

  ClikeLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: ClikeLexer},
    didShrinkLastToken: {value: didShrinkLastToken },
    nextToken: {value: nextToken},
    syntaxOfToken: {value: syntaxOfToken}
  });

  return ClikeLexer;
})();
