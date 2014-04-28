// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.CppLexer = (function() {
  /** @enum{number} */
  var State = {
    ZERO: 0, // State.ZERO must be zero.

    BLOCK_COMMENT: 3,
    BLOCK_COMMNNT_ASTERISK: 4,
    BLOCK_COMMNNT_SLASH: 5,
    LINE_COMMENT: 6,
    OPERATOR: 7,
    SLASH: 8,
    SPACE: 9,
    STRING1: 10,
    STRING1_ESCAPE: 11,
    STRING2: 12,
    STRING2_ESCAPE: 13,
    WORD: 14
  };

  /** @enum{string} */
  var Token = {
    BLOCK_COMMENT: 'BLOCK_COMMENT',
    LINE_COMMENT: 'LINE_COMMENT',
    NEWLINE: 'NEWLINE',
    OPERATOR: 'OPERATOR',
    SPACE: 'SPACE',
    STRING: 'STRING',
    WORD: 'WORD',
  };

  var SyntaxMap = {
    Token.BLOCK_COMMENT: 'comment',
    Token.LINE_COMMENT: 'comment',
    Token.OPERATOR: 'operators',
    Token.STRING: 'string_literal',
    Token.WORD: 'identifier',
  };

  /** @const @type {Set.<string>} */
  var KEYWORDS = (function() {
    var set = new Set();
    [
      'do', 'for', 'if',
    ].forEach(function(keyword) {
      set.add(keyword);
    });
    return set;
  })();

  /**
   * @constructor
   * @extends Lexer
   * @param {!Document} document
   */
  function CppLexer(document) {
    Lexer.call(this, document);
  }

  /**
   * @this {!CppLexer}
   * @param {number} maxCount
   */
  function doColor(maxCount) {
    if (!this.range)
      throw new Error("Can't use disconnected lexer.");

    this.adjustScanOffset(maxCount);
    var range = this.range;
    var document = range.document;
    var maxOffset = document.length;
    while (this.scanOffset < maxOffset) {
      var token = nextToken(this);
      if (!token)
        break;
      if (range.collapsed)
        continue;
      if (this.debug_ > 0)
        console.log('CppLexer', token, range);
      var syntax = SYNTAX_MAP[token.type] || 'normal';
      if (token.type == Token.WORD) {
        var command = range.text;
       if (KEYWORDS.has(command.toLowerCase()))
          syntax = 'keyword';
      }
      range.setSyntax(syntax);
    }

    return this.count;
  }

  /**
   * @type {number} charCode
   * @return {boolean}
   */
  function isWordFirst(charCode) {
    if (charCode >= Unicode.LATIN_CAPITAL_LETTER_A &&
        charCode <= Unicode.LATIN_CAPITAL_LETTER_Z) {
      return true;
    }

    if (charCode >= Unicode.LATIN_SMALL_LETTER_A &&
        charCode <= Unicode.LATIN_SMALL_LETTER_Z) {
      return true;
    }

    return charCode == Unicode.LOW_LINE;
  }

  /**
   * @type {number} charCode
   * @return {boolean}
   */
  function isOperator(charCode) {
    switch (charCode) {
      case Unicode.LF:
      case Unicode.SPACE:
      case Unicode.TAB:
        return false;
      default:
        return isWordRest(charCode);
    }
  }

  /**
   * @type {number} charCode
   * @return {boolean}
   */
  function isWordRest(charCode) {
    if (isWordFirst(charCode))
      return true;
    return charCode >= Unicode.DIGIT_ZERO && charCode <= Unicode.DIGIT_NINE;
  }

  /**
   * @param {!CppLexer} lexer
   * @return {?string}
   */
  function nextToken(lexer) {
    if (!lexer.count)
      return null;
    var document = lexer.range.document;
    var maxOffset = document.length;
    if (lexer.scanOffset == maxOffset)
      return null;
    while (lexer.count) {
      --lexer.count;
      if (lexer.scanOffset == maxOffset)
        break;
      var charCode = document.charCodeAt_(lexer.scanOffset);
      ++lexer.scanOffset;
      switch (lexer.state) {
        case State.BLOCK_COMMENT:
          switch (charCode) {
            case Unicode.ASTERISK:
              lexer.state = State.BLOCK_COMMENT_ASTERISK;
              break;
            case Unicode.SLASH:
              lexer.state = State.BLOCK_COMMENT_SLASH;
              break;
          }
          break;

        case State.BLOCK_COMMENT_ASTERISK:
          switch (charCode) {
            case Unicode.ASTERISK:
              lexer.state = State.BLOCK_COMMENT_ASTERISK;
              break;
            case Unicode.SLASH:
              --lexer.lastToken_.depth;
              if (!lexer.lastToken_.depth)
                return lexer.finishToken();
              lexer.state = State.BLOCK_COMMENT;
              break;
            default:
              lexer.state = State.BLOCK_COMMENT;
              break;
          }
          break;

        case State.BLOCK_COMMENT_SLASH:
          switch (charCode) {
            case Unicode.ASTERISK:
              ++lexer.lastToken_.depth;
              lexer.state = State.BLOCK_COMMENT;
              break;
            case Unicode.SLASH:
              break;
            default:
              lexer.state = State.BLOCK_COMMENT;
              break;
          }
          break;

        case State.LINE_COMMENT:
          if (charCode == Unicode.LF) {
            --lexer.scanOffset;
            return lexer.finishToken();
          }
          break;

        case State.OPERATOR:
          if (!isOperator(charCode)) {
              --lexer.scanOffset;
              return lexer.finishToken();
          }
          break;

        case State.SLASH:
          switch (charCode) {
            case Unicode.ASTERISK:
              lexer.state = State.BLOCK_COMMENT;
              break;
            case Unicode.SLASH:
              lexer.state = State.LINE_COMMENT;
              break;
            default:
              if (!isOperator(charCode)) {
                --lexer.scanOffset;
                return lexer.finishToken();
              }
              lexer.state = State.OPERATOR;
              break;
            }
            break;

        case State.STRING1:
          if (charCode == Unicode.APOSTROPHE)
            return lexer.finishToken();
          if (charCode == Unicode.REVERSE_SOLIDUS)
            lexer.state = State.STRING1_ESCAPE;
          break;
        case State.STRING1_ESCAPE:
          lexer.state = State.STRING1;
          break;

        case State.STRING2:
          if (charCode == Unicode.QUOTATION_MARK)
            return lexer.finishToken();
          if (charCode == Unicode.REVERSE_SOLIDUS)
            lexer.state = State.STRING2_ESCAPE;
          break;
        case State.STRING2_ESCAPE:
          lexer.state = State.STRING2;
          break;

        case State.WORD:
          if (isWordRest(charCode))
            break;
          --lexer.scanOffset;
          return lexer.finishToken();

        case State.ZERO:
          switch (charCode) {
            case Unicode.APOSTROPHE:
              lexer.startToken(State.STRING1, Token.STRING);
              break;
            case Unicode.LF:
              lexer.startToken(State.ZERO, Token.NEWLINE);
              return lexer.finishToken();
            case Unicode.NUMBER_SIGN:
              if (lexer.lastToken_.type == Token.NEWLINE) {
                lexer.startToken(State.ZERO, Token.NUMBER_SIGN);
                return lexer.finishToken();
              }
              lexer.startToken(State.OPERATOR, Token.OPERATOR);
              break;
            case Unicode.QUOTATION_MARK:
              lexer.startToken(State.STRING2, Token.STRING);
              break;
            case Unicode.SLASH:
              lexer.state = State.SLASH;
              break;
            case Unicode.SPACE:
            case Unicode.TAB:
              lexer.startToken(State.SPACE, Token.SPACE);
              break;
            default:
              if (isWordRest(charCode))
                lexer.startToken(State.WORD, Token.WORD);
              else
                lexer.startToken(State.OPERATOR, Token.OPERATOR);
              break;
          }
          break;
        default:
          throw new Error('Invalid state ' + lexer.state);
      }
    }
    lexer.range.end = lexer.scanOffset;
    return lexer.syntax;
  }

  CppLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: CppLexer},
    doColor: {value: doColor},
  });

  return CppLexer;
})();
