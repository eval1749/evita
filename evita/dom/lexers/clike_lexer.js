// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.ClikeLexer = (function() {
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
    map.set(Unicode.SOLIDUS, ClikeLexer.SLASH_CHAR);

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
    LINE_COMMENT_START: Symbol('line_commend_start'),
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
   * @return {!Lexer.State}
   */
  function didShrinkLastToken(token) {
    if (this.debug_ > 1)
      console.log('didShrinkLastToken', token);
    switch (token.state) {
      case ClikeLexer.State.BLOCK_COMMENT_END:
        return ClikeLexer.State.BLOCK_COMMENT_ASTERISK;
      case ClikeLexer.State.BLOCK_COMMENT_START:
        return ClikeLexer.State.SOLIDUS;
      case ClikeLexer.State.COLON_COLON:
        return ClikeLexer.State.COLON;
      case Lexer.State.DOT:
        return Lexer.State.ZERO;
      case ClikeLexer.State.LINE_COMMENT_START:
        return ClikeLexer.State.SOLIDUS;
    }
    return token.state;
  }

  /**
   * @this {!ClikeLexer}
   * @param {number} charCode
   */
  function feedCharacter(charCode) {
    switch (this.state) {
      case ClikeLexer.State.BLOCK_COMMENT:
        if (charCode == Unicode.ASTERISK)
          this.finishToken(ClikeLexer.State.BLOCK_COMMENT_ASTERISK);
        else
          this.extendToken();
        return;

      case ClikeLexer.State.BLOCK_COMMENT_ASTERISK:
        if (charCode == Unicode.ASTERISK)
          this.extendToken();
        else if (charCode == Unicode.SOLIDUS)
          this.restartToken(ClikeLexer.State.BLOCK_COMMENT_END);
        else
          this.finishToken(ClikeLexer.State.BLOCK_COMMENT);
        return;

      case ClikeLexer.State.BLOCK_COMMENT_END:
        this.state = Lexer.State.ZERO;
        return;

      case ClikeLexer.State.BLOCK_COMMENT_START:
        if (charCode == Unicode.ASTERISK)
          this.startToken(ClikeLexer.State.BLOCK_COMMENT_ASTERISK);
        else
          this.startToken(ClikeLexer.State.BLOCK_COMMENT);
        return;

      case ClikeLexer.State.COLON:
        if (charCode == Unicode.COLON)
          this.restartToken(ClikeLexer.State.COLON_COLON);
        else
          this.endToken();
        return;

      case ClikeLexer.State.COLON_COLON:
        this.state = Lexer.State.ZERO;
        return;

      case ClikeLexer.State.LINE_COMMENT:
        if (charCode == Unicode.LF)
          this.endToken();
        else if (charCode == Unicode.REVERSE_SOLIDUS)
          this.finishToken(ClikeLexer.State.LINE_COMMENT_ESCAPE);
        else
          this.extendToken();
        return;

      case ClikeLexer.State.LINE_COMMENT_ESCAPE:
        this.finishToken(ClikeLexer.State.LINE_COMMENT);
        return;

      case ClikeLexer.State.LINE_COMMENT_START:
        if (charCode == Unicode.LF) {
          this.state = Lexer.State.ZERO;
          return;
        }
        if (charCode == Unicode.REVERSE_SOLIDUS)
          this.startToken(ClikeLexer.State.LINE_COMMENT_ESCAPE);
        else
          this.startToken(ClikeLexer.State.LINE_COMMENT);
        return;

      case ClikeLexer.State.NEWLINE:
        if (this.isWhitespaceChar(charCode))
          this.extendToken();
        else
          this.endToken();
        return;

      case ClikeLexer.State.NUMBER_SIGN:
        this.state = Lexer.State.ZERO;
        return;

      case ClikeLexer.State.SOLIDUS:
        if (charCode == Unicode.ASTERISK)
          this.restartToken(ClikeLexer.State.BLOCK_COMMENT_START);
        else if (charCode == Unicode.SOLIDUS)
          this.restartToken(ClikeLexer.State.LINE_COMMENT_START);
        else
          this.endToken();
        return;

      case Lexer.State.ZERO:
        switch (charCode) {
          case Unicode.LF:
            this.startToken(ClikeLexer.State.NEWLINE);
            return;
          case Unicode.SOLIDUS:
            this.startToken(ClikeLexer.State.SOLIDUS);
            return;
          default:
            if (this.hasCpp && charCode == Unicode.NUMBER_SIGN) {
              if (!this.lastToken ||
                  this.lastToken.state == ClikeLexer.State.NEWLINE) {
                this.startToken(ClikeLexer.State.NUMBER_SIGN);
                this.endToken();
                return;
              }
              this.startToken(Lexer.State.OPERATOR);
              return;
            }

            if (this.useColonColon && charCode == Unicode.COLON) {
              this.startToken(ClikeLexer.State.COLON);
              return;
            }

            if (this.useDot && charCode == Unicode.FULL_STOP) {
              this.startToken(Lexer.State.DOT);
              this.endToken();
              return;
            }
            break;
        }
        break;
    }
    this.updateState(charCode);
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
    feedCharacter: {value: feedCharacter},
    syntaxOfToken: {value: syntaxOfToken}
  });

  return ClikeLexer;
})();
