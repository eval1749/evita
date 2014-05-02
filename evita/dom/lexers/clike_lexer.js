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
  var StateToSyntax = new Map();
  StateToSyntax.set(State.ZERO, 'normal');
  StateToSyntax.set(State.BLOCK_COMMENT, 'comment');
  StateToSyntax.set(State.BLOCK_COMMENT_ASTERISK, 'comment');
  StateToSyntax.set(State.BLOCK_COMMENT_END, 'comment');
  StateToSyntax.set(State.BLOCK_COMMENT_START, 'comment');
  StateToSyntax.set(State.COLON, 'operators');
  StateToSyntax.set(State.COLON_COLON, 'operators');
  StateToSyntax.set(State.LINE_COMMENT, 'comment');
  StateToSyntax.set(State.LINE_COMMENT_ESCAPE, 'comment');
  StateToSyntax.set(State.LINE_COMMENT_START, 'comment');
  StateToSyntax.set(State.NEWLINE, 'normal');
  StateToSyntax.set(State.NUMBER_SIGN, 'operators');
  StateToSyntax.set(State.OPERATOR, 'operators');
  StateToSyntax.set(State.SOLIDUS, 'operators');
  StateToSyntax.set(State.SPACE, 'normal');
  StateToSyntax.set(State.STRING1, 'string_literal');
  StateToSyntax.set(State.STRING1_END, 'string_literal');
  StateToSyntax.set(State.STRING1_ESCAPE, 'string_literal');
  StateToSyntax.set(State.STRING2, 'string_literal');
  StateToSyntax.set(State.STRING2_END, 'string_literal');
  StateToSyntax.set(State.STRING2_ESCAPE, 'string_literal');
  StateToSyntax.set(State.WORD, 'identifier');

  Object.keys(State).forEach(function(key) {
    if (!StateToSyntax.has(State[key]))
      throw new Error('StateToSyntax must have ' + key);
  });

  /**
   * @constructor
   * @extends Lexer
   * @param {!Document} document
   * @param {!ClikeLexerOptions} options
   */
  function ClikeLexer(document, options) {
    Lexer.call(this, options.keywords, document);
  }

  /**
   * @this {!ClikeLexer}
   * @param {number} maxCount
   */
  function doColor(maxCount) {
    if (!this.range)
      throw new Error("Can't use disconnected lexer.");

    this.adjustScanOffset();
    var document = this.range.document;
    var maxOffset = Math.min(this.scanOffset + maxCount, document.length);
    this.count = maxCount;
    while (this.scanOffset < maxOffset) {
      var token = nextToken(this);
      if (!token)
        break;
      setSyntax(this, token);
      if (this.lastToken != token)
        setSyntax(this, /** @type {!Lexer.Token} */(this.lastToken));
    }
    return this.count;
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
    if (token.state == State.LINE_COMMENT_START) {
      token.state = State.SOLIDUS;
      return;
    }
  }

  /**
   * @param {!ClikeLexer} lexer
   * @param {!Range} range
   * @param {!Lexer.Token} token
   * @return {string}

   *
   * Extract CPP directive as '#" + word or namespace qualify identifier.
   */
  function extractWord(lexer, range, token) {
    var word = range.text;
    var it = lexer.tokens.find(token);
    console.assert(it, token);
    do {
      it = it.previous();
    } while (it && it.data.state == State.SPACE);

    if (!it)
      return word;

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
   * @param {number} charCode
   * @return {boolean}
   */
  function isOperator(charCode) {
    return !isWhitespace(charCode) && !isWordRest(charCode) &&
           charCode != Unicode.APOSTROPHE &&
           charCode != Unicode.QUOTATION_MARK;
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
   * @param {number} charCode
   * @return {boolean}
   */
  function isWordRest(charCode) {
    if (isWordFirst(charCode))
      return true;
    return charCode >= Unicode.DIGIT_ZERO && charCode <= Unicode.DIGIT_NINE;
  }

  /**
   * @param {!ClikeLexer} lexer
   * @return {?Lexer.Token}
   */
  function nextToken(lexer) {
    if (!lexer.count)
      return null;
    var document = lexer.range.document;
    var maxOffset = document.length;
    if (lexer.scanOffset == maxOffset)
      return null;
    while (lexer.scanOffset < maxOffset && lexer.count) {
      --lexer.count;
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
          if (!isWhitespace(charCode))
            return lexer.finishToken(State.ZERO);
          lexer.extendToken();
          break;

        case State.OPERATOR:
          if (!isOperator(charCode))
            return lexer.finishToken(State.ZERO);
          lexer.extendToken();
          break;

        case State.SOLIDUS:
          if (charCode == Unicode.ASTERISK)
            return lexer.finishTokenAs(State.BLOCK_COMMENT_START);
          if (charCode == Unicode.SOLIDUS)
            return lexer.finishTokenAs(State.LINE_COMMENT_START);
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
          if (!isWordRest(charCode))
            return lexer.finishToken(State.ZERO);
          lexer.extendToken();
          break;

        case State.ZERO:
          switch (charCode) {
            case Unicode.APOSTROPHE:
              lexer.startToken(State.STRING1);
              break;
            case Unicode.COLON:
              lexer.startToken(State.COLON);
              break;
            case Unicode.LF:
              lexer.startToken(State.NEWLINE);
              break;
            case Unicode.NUMBER_SIGN:
              if (!lexer.lastToken || lexer.lastToken.state == State.NEWLINE) {
                lexer.startToken(State.NUMBER_SIGN);
                return lexer.finishToken(State.ZERO);
              }
              lexer.startToken(State.OPERATOR);
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
              if (isWordRest(charCode))
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

  /**
   * @param {!ClikeLexer} lexer
   * @param {!Lexer.Token} token
   */
  function setSyntax(lexer, token) {
    var range = lexer.range;
    range.collapseTo(token.start);
    range.end = token.end;
    var syntax = StateToSyntax.get(token.state) || 'normal';
    if (syntax == 'identifier') {
      var word = extractWord(lexer, range, token);
      if (lexer.debug_ > 5)
        console.log('setSyntax', '"' + word + '"');
      if (lexer.keywords.has(word))
        syntax = 'keyword';
    }
    if (lexer.debug_ > 4)
      console.log('setSyntax', syntax, token);
    range.setSyntax(syntax);
  }

  ClikeLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: ClikeLexer},
    didShrinkLastToken: {value: didShrinkLastToken },
    doColor: {value: doColor}
  });

  return ClikeLexer;
})();
