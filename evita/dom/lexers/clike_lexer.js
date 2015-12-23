// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  // Maximum number of words in symbol, e.g. Foo::Bar::Baz
  const MAX_WORDS_IN_SYMBOL = 3;

  //////////////////////////////////////////////////////////////////////
  //
  // ClikeLexer
  //
  class ClikeLexer extends global.Lexer {
    /**
     * @param {!Document} document
     * @param {!ClikeLexerOptions} options
     */
    constructor(document, options) {
      super(document, {
        characters: options.characters,
        keywords: options.keywords
      });
      this.hasCpp = options.hasCpp;
      this.useColonColon = options.useColonColon;
      this.useDot = options.useDot;
    }

    /** @override */
    didEndToken(token) {
      switch (token.state) {
        case ClikeLexer.State.COLON:
          return handleColonToken(this, token);
        case lexers.State.WORD:
          return handleWordToken(this, token);
      }
    }

    /** * @override */
    syntaxOfToken(token) {
      return STATE_TO_SYNTAX.get(token.state) || super.syntaxOfToken(token);
    }
  }

  /** @enum{!symbol} */
  ClikeLexer.State = {
    BLOCK_COMMENT: Symbol('block_comment'),
    BLOCK_COMMENT_ASTERISK: Symbol('block_comment_asterisk'),
    BLOCK_COMMENT_END: Symbol('block_comment_end'),
    BLOCK_COMMENT_START: Symbol('block_comment_start'),
    COLON: Symbol(':'),
    COLON_COLON: Symbol('::'),
    LINE_COMMENT: Symbol('line_comment'),
    LINE_COMMENT_ESCAPE: Symbol('line_comment_escape'),
    LINE_COMMENT_START: Symbol('line_commend_start'),
    NEWLINE: Symbol('newline'),
    NUMBER_SIGN: Symbol('#'),
    SOLIDUS: Symbol('/'),
  };

  ClikeLexer.COLON_CHAR = Symbol('COLON_CHAR');
  ClikeLexer.DOT_CHAR = Symbol('DOT_CHAR');
  ClikeLexer.SLASH_CHAR = Symbol('SLASH_CHAR');

  /** @const @type {!Map.<number, !symbol>} */
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
   * @this {!ClikeLexer}
   * @param {!lexers.Token} token
   * @return {!lexers.State}
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
      case lexers.State.DOT:
        return lexers.State.ZERO;
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
          this.finishState(ClikeLexer.State.BLOCK_COMMENT_ASTERISK);
        else
          this.extendToken();
        return;

      case ClikeLexer.State.BLOCK_COMMENT_ASTERISK:
        if (charCode == Unicode.ASTERISK)
          this.extendToken();
        else if (charCode == Unicode.SOLIDUS)
          this.restartToken(ClikeLexer.State.BLOCK_COMMENT_END);
        else
          this.finishState(ClikeLexer.State.BLOCK_COMMENT);
        return;

      case ClikeLexer.State.BLOCK_COMMENT_END:
        this.state = lexers.State.ZERO;
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
        this.state = lexers.State.ZERO;
        return;

      case ClikeLexer.State.LINE_COMMENT:
        if (charCode == Unicode.LF)
          this.endToken();
        else if (charCode == Unicode.REVERSE_SOLIDUS)
          this.finishState(ClikeLexer.State.LINE_COMMENT_ESCAPE);
        else
          this.extendToken();
        return;

      case ClikeLexer.State.LINE_COMMENT_ESCAPE:
        this.finishState(ClikeLexer.State.LINE_COMMENT);
        return;

      case ClikeLexer.State.LINE_COMMENT_START:
        if (charCode == Unicode.LF) {
          this.state = lexers.State.ZERO;
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
        this.state = lexers.State.ZERO;
        return;

      case ClikeLexer.State.SOLIDUS:
        if (charCode == Unicode.ASTERISK)
          this.restartToken(ClikeLexer.State.BLOCK_COMMENT_START);
        else if (charCode == Unicode.SOLIDUS)
          this.restartToken(ClikeLexer.State.LINE_COMMENT_START);
        else
          this.endToken();
        return;

      case lexers.State.ZERO:
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
              this.startToken(lexers.State.OPERATOR);
              return;
            }

            if (charCode == Unicode.COLON) {
              this.startToken(ClikeLexer.State.COLON);
              if (!this.useColonColon)
                this.endToken();
              return;
            }

            if (this.useDot && charCode == Unicode.FULL_STOP) {
              this.startToken(lexers.State.DOT);
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
   * @param {base.OrderedSetNode.<!lexers.Token>} startNode
   * @param {number} maxWords
   * @return {!Array.<!lexers.Token>}
   *
   * Collects words separated by '::' or '.'.
   */
  function extractSymbol(startNode, maxWords) {
    const tokens = [];
    let node = startNode;
    while (node) {
      const token = node.data;
      if (token.state !== lexers.State.WORD)
        break;
      tokens.push(token);
      --maxWords;
      if (maxWords === 0)
        break;
      do {
        node = node.previous();
      } while (node && node.data.state === lexers.State.SPACE);
      if (!node)
        break;
      if (!isNsSeparator(node.data))
        break;
      tokens.push(node.data);
      do {
        node = node.previous();
      } while (node && node.data.state === lexers.State.SPACE);
      if (!node)
        break;
    }
    return tokens.reverse();
  }

  /**
   * @param {!ClikeLexer} lexer
   * @param {!lexers.Token} colonToken
   * Marks word token before colon to label.
   */
  function handleColonToken(lexer, colonToken) {
    console.assert(colonToken.state === ClikeLexer.State.COLON, colonToken);
    const colonNode = lexer.lowerBound(colonToken.start + 1);
    const tokens = extractSymbol(colonNode.previous(), MAX_WORDS_IN_SYMBOL);
    if (tokens.length === 0)
      return;
    const text = tokens.map(token => lexer.tokenTextOf(token)).join('');
    const type = lexer.keywords.get(text + ':') ? 'keyword' : 'label';
    tokens.forEach(token => lexer.changeTokenType(token, type));
  }

  /**
   * @param {!ClikeLexer} lexer
   * @param {!lexers.Token} wordToken
   */
  function handleWordToken(lexer, wordToken) {
    console.assert(wordToken.state === lexers.State.WORD, wordToken);
    const wordIt = lexer.lowerBound(wordToken.start + 1);
    let sharpIt = wordIt.previous();

    while (sharpIt && sharpIt.data.state === lexers.State.SPACE)
      sharpIt = sharpIt.previous();

    if (sharpIt && sharpIt.data.state === ClikeLexer.State.NUMBER_SIGN) {
      const text = lexer.tokenTextOf(wordToken);
      const type = lexer.keywords.get('#' + text) || '';
      lexer.changeTokenType(sharpIt.data, type);
      lexer.changeTokenType(wordToken, type);
      return;
    }

    const tokens = extractSymbol(wordIt, MAX_WORDS_IN_SYMBOL);
    const text = tokens.map(token => lexer.tokenTextOf(token)).join('');
    const type = lexer.keywords.get(text) || '';
    if (type !== '' || tokens.length === 1)
      return tokens.forEach(token => lexer.changeTokenType(token, type));
    // Handle reserved property
    const dotName = tokens.slice(-2);
    if (dotName[0].state !== lexers.State.DOT)
      return tokens.forEach(token => lexer.changeTokenType(token, ''));
    const dotNameText = '.' + lexer.tokenTextOf(wordToken);
    const dotNameType = lexer.keywords.get(dotNameText) || '';
    dotName.forEach(token => lexer.changeTokenType(token, dotNameType));
  }

  /**
   * @param {!lexers.Token} token
   * @return {boolean}
   */
  function isNsSeparator(token) {
    return token.state === ClikeLexer.State.COLON_COLON ||
           token.state === lexers.State.DOT;
  }

  ClikeLexer.newCharacters = function() {
    var map = new Map();
    CHARACTERS.forEach(function(value, key) {
      map.set(key, value);
    });
    return map;
  };

  Object.defineProperties(ClikeLexer.prototype, {
    didShrinkLastToken: {value: didShrinkLastToken },
    feedCharacter: {value: feedCharacter},
  });

  global['ClikeLexer'] = ClikeLexer;
})();
