// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'lexers', function($export) {
  // For ease of restarting lexer, we have string escape and string end tokens
  // to avoid backward scanning to determine escape character and string ending
  // character.
  // For example, when buffer has just string "foo", including double quote,
  // we have following tokens:
  //    STRING2 [0, 3] "12
  //    STRING2_END [3, 4] "
  // Typing "x" at offset 4, |doColor()| starts with |STRING2_END|, then
  // moves to |ZERO|.
  const State = {
    DOT: Symbol('.'),
    LINE_COMMENT: Symbol('line_comment'),
    OPERATOR: Symbol('operator'),
    OTHER: Symbol('other'),
    SPACE: Symbol('space'),
    STRING1: Symbol('string1'),
    STRING1_END: Symbol('string1_end'),
    STRING1_ESCAPE: Symbol('string1_escape'),
    STRING2: Symbol('string2'),
    STRING2_END: Symbol('string2_end'),
    STRING2_ESCAPE: Symbol('string2_escape'),
    STRING3: Symbol('string3'),
    STRING3_END: Symbol('string3_end'),
    STRING3_ESCAPE: Symbol('string3_escape'),
    WORD: Symbol('word'),
    ZERO: Symbol('zero'),
  };

  class Token {
    /**
     * @param {!lexers.State} state
     * @param {number} start
     */
    constructor(state, start) {
      this.end = start + 1;
      this.start = start;
      this.state = state;
      this.type = '';
    }
  }

  $export({State, Token});
});

/**
 * Note: Closure compiler doesn't allow to write |function Lexer|, we use
 * IIFE to set constructor name to |Lexer| rather than |global.Lexer|.
 */
(function() {
  //////////////////////////////////////////////////////////////////////
  //
  // Lexer
  //
  class Lexer extends text.SimpleMutationObserverBase {
    /**
     * @param {!TextDocument} document
     * @param {!LexerOptions} options
     * @return {undefined}
     */
    constructor(document, options) {
      super(document);
      this.characters_ = options.characters;
      this.debug_ = 0;
      /** @type {number} */
      this.dirtyTokenStart_ = 0;
      /** @const @type {!lexers.Token} */
      this.dummyToken_ = new lexers.Token(lexers.State.ZERO, 0);
      this.keywords = options.keywords;
      this.lastToken = this.dummyToken_;
      // TODO(eval1749): |maxChainWords_| should be part of |LexerOptions|.
      this.maxChainWords_ = 3;
      this.range = new TextRange(document);
      this.scanOffset = 0;
      this.state = lexers.State.ZERO;
      this.tokens = new base.OrderedSet((a, b) => a.end < b.end);
    }

    /**
     * @param {!lexers.Token} token
     * @param {string} type
     */
    changeTokenType(token, type) {
      if (token.type === type)
        return;
      token.type = type;
      this.didChangeToken(token);
    }

    /**
     * @private
     * Implements text.SimpleMutationObserver.didChangeTextDocument
     * @param {number} offset
     */
    didChangeTextDocument(offset) {
      const delta = offset - this.scanOffset;
      if (delta > 0) {
        this.doColor(delta);
      } else {
        this.adjustScanOffset(offset);
        this.doColor(100);
      }
      if (this.scanOffset >= this.document.length)
        return;
      taskScheduler.schedule(this, 500);
    }

    /**
     * @param {!lexers.Token} token
     */
    didChangeToken(token) {
      this.dirtyTokenStart_ = Math.min(this.dirtyTokenStart_, token.start);
    }

    /** @param {!lexers.Token} token */
    didEndToken(token) {}

    /**
     * @override
     * Implements text.SimpleMutationObserver.didChangeTextDocument
     */
    didLoadTextDocument() {
      this.adjustScanOffset(0);
      this.doColor(this.document.length);
    }

    /**
     * @param {!lexers.Token} token
     * @return {!lexers.State}
     */
    didShrinkLastToken(token) { return token.state; }

    /**
     * @param {number} offset
     * @return {OrderedSetNode<!lexers.Token>}
     */
    lowerBound(offset) {
      this.dummyToken_.end = offset;
      return this.tokens.lowerBound(this.dummyToken_);
    }

    run() {
      /** @type {number}
       * Number of characters to color during scheduled task.
       * This is an experiment for searching right value.
       */
      const kIncrementalCount = 1000;
      this.doColor(kIncrementalCount);
      if (this.scanOffset >= this.document.length)
        return;
      // Continue coloring after one second.
      taskScheduler.schedule(this, 500);
    }

    /**
     * @param {!lexers.Token} token
     * @return {string}
     */
    tokenTextOf(token) { return this.document.slice(token.start, token.end); }

    /**
     * @param {!Iterable<string>|!Array<string>} keywords
     * @return {!Map.<string, string>}
     */
    static createKeywords(keywords) {
      const map = new Map();
      for (const keyword of keywords)
        map.set(keyword, 'keyword');
      return map;
    }
  }

  Lexer.DOT_CHAR = Symbol('dot');
  Lexer.NAME_CHAR = Symbol('name');
  Lexer.NAMESTART_CHAR = Symbol('nameStart');
  Lexer.OPERATOR_CHAR = Symbol('operator');
  Lexer.OTHER_CHAR = Symbol('other');
  Lexer.WHITESPACE_CHAR = Symbol('whitespace');
  Lexer.STRING1_CHAR = Symbol('string1');
  Lexer.STRING2_CHAR = Symbol('string2');
  Lexer.STRING3_CHAR = Symbol('string3');

  /**
   * @this {!Lexer}
   * @param {number} changedOffset
   */
  function adjustScanOffset(changedOffset) {
    if (!this.lastToken)
      return;
    const document = this.document;
    const newScanOffset = Math.min(changedOffset, this.lastToken.end);
    if (this.debug_ > 0)
      console.log('adjustScanOffset', newScanOffset, this);

    this.scanOffset = newScanOffset;
    if (newScanOffset === null || this.tokens.empty() ||
        newScanOffset <= this.tokens.minimum.start) {
      // All tokens in token list are dirty.
      this.clear();
      return;
    }

    let it = this.lowerBound(newScanOffset);
    console.assert(it, newScanOffset);

    if (it.data.start >= newScanOffset) {
      it = it.previous();
      if (it === null) {
        this.clear();
        return;
      }
    }

    const lastToken = it.data;

    // Remove dirty tokens
    for (;;) {
      const maximum = this.tokens.maximum;
      if (maximum === lastToken)
        break;
      this.tokens.remove(maximum);
    }

    if (lastToken.end > newScanOffset) {
      lastToken.end = newScanOffset;
      lastToken.state = this.didShrinkLastToken(lastToken);
    }

    this.lastToken = lastToken;
    this.state = lastToken.state;
    this.tokenData = lastToken.data;
  }

  /**
   * @this {!Lexer}
   */
  function clear() {
    if (this.debug_ > 0)
      console.log('Lexer.clear', this);
    this.lastToken = this.dummyToken_;
    this.scanOffset = 0;
    this.state = lexers.State.ZERO;
    this.tokens.clear();
  }

  /**
   * @this {!Lexer}
   * @param {!base.OrderedSetNode.<!lexers.Token>} itDelimiter
   * @param {!lexers.Token} token
   * @return {!Array.<!lexers.Token>}
   */
  function collectTokens(itDelimiter, token) {
    let delimiter = itDelimiter.data;
    let tokens = [token];
    let it = itDelimiter;
    while (it && it.data.state === delimiter.state &&
           tokens.length < this.maxChainWords_) {
      tokens.unshift(it.data);
      do {
        it = it.previous();
      } while (it && it.data.state === lexers.State.SPACE);
      if (!it || it.data.state !== lexers.State.WORD)
        break;
      tokens.unshift(it.data);
      it = it.previous();
    }
    return tokens;
  }

  /**
   * @this {!Lexer}
   * @param {!lexers.Token} token
   */
  function colorToken(token) {
    if (token.type != '') {
      this.document.setSyntax(token.start, token.end, token.type);
      return;
    }
    let syntax = this.syntaxOfToken(token) || '';
    if (this.debug_ > 4)
      console.log(`setSyntax "${syntax}"`, token);
    this.document.setSyntax(
        token.start, token.end, syntax === 'identifier' ? '' : syntax);
  }

  /**
   * @this {!Lexer}
   */
  function detach() {
    if (!this.range)
      throw new Error(`${this} isn't attached to document.`);
    this.stopObserving();
    this.range = null;
  }

  /**
   * @this {!Lexer}
   * @param {number} maxCount
   */
  function doColor(maxCount) {
    if (!this.range)
      throw new Error("Can't use disconnected lexer.");
    const document = this.document;
    const maxOffset = Math.min(this.scanOffset + maxCount, document.length);
    this.dirtyTokenStart_ = this.scanOffset;
    while (this.scanOffset < maxOffset) {
      let charCode = document.charCodeAt(this.scanOffset);
      this.feedCharacter(charCode);
    }
    const startOffset = this.dirtyTokenStart_;
    const endOffset = this.scanOffset;
    this.dirtyTokenStart_ = this.scanOffset;
    // Color tokens
    /** @type {OrderedSetNode<!lexers.Token>} */
    const startTokenIt = this.lowerBound(startOffset);
    if (!startToken)
      return;
    /** @type {OrderedSetNode<!lexers.Token>} */
    const endTokenIt = this.lowerBound(endOffset);
    /** @type {OrderedSetNode<!lexers.Token>} */
    let tokenIt = startTokenIt;
    while (tokenIt != endTokenIt) {
      this.colorToken(tokenIt.data);
      tokenIt = tokenIt.next();
    }
    if (this.lastToken.state === lexers.State.ZERO)
      return;
    if (this.debug_ > 2)
      console.log('doColor', 'lastToken', this.lastToken);
    this.colorToken(this.lastToken);
  }

  /**
   * @this {!Lexer}
   * End lexical token.
   * Note: lexical token is consists with multiple |Lexer.Token| objects.
   */
  function endToken() {
    this.didEndToken(this.lastToken);
    this.state = lexers.State.ZERO;
  }

  /**
   * @this {!Lexer}
   */
  function extendToken() {
    ++this.scanOffset;
    this.lastToken.end = this.scanOffset;
  }

  /**
   * @this {!Lexer}
   * @param {!lexers.State} nextState
   * Moves to new state.
   * Note: lexical token has not been ended.
   */
  function finishState(nextState) {
    if (this.debug_ > 2)
      console.log('finishState', this.lastToken);
    this.startToken(nextState);
  }

  /**
   * @this {!Lexer}
   * @param {number} charCode
   * @return {boolean}
   */
  function isNameChar(charCode) {
    let type = this.characters_.get(charCode);
    return type === Lexer.NAMESTART_CHAR || type === Lexer.NAME_CHAR;
  }

  /**
   * @this {!Lexer}
   * @param {number} charCode
   * @return {boolean}
   */
  function isNameStartChar(charCode) {
    return this.characters_.get(charCode) === Lexer.NAMESTART_CHAR;
  }

  /**
   * @this {!Lexer}
   * @param {number} charCode
   * @return {boolean}
   */
  function isOtherChar(charCode) { return !this.characters_.has(charCode); }

  /**
   * @this {!Lexer}
   * @param {number} charCode
   * @return {boolean}
   */
  function isWhitespaceChar(charCode) {
    return this.characters_.get(charCode) === Lexer.WHITESPACE_CHAR;
  }

  /**
   * @this {!Lexer}
   * @param {number} charCode
   * @return {boolean}
   */
  function isWordChar(charCode) {
    let type = this.characters_.get(charCode);
    return type === Lexer.NAMESTART_CHAR || type === Lexer.NAME_CHAR;
  }

  /**
   * @this {!Lexer}
   * @param {!lexers.State} newState
   */
  function restartToken(newState) {
    console.assert(
        newState !== lexers.State.ZERO, 'newState must not be zero.');
    this.extendToken();
    if (this.debug_ > 2)
      console.log('restartToken', newState, this.lastToken);
    this.state = newState;
    this.lastToken.state = newState;
  }

  /**
   * @this {!Lexer}
   * @param {!lexers.State} state
   */
  function startToken(state) {
    console.assert(state !== lexers.State.ZERO, 'state must not be zero.');
    if (this.debug_ > 2)
      console.log('startToken', state, this.scanOffset);
    let token = new lexers.Token(state, this.scanOffset);
    token.data = this.tokenData;
    if (this.debug_ > 0)
      console.assert(!this.tokens.find(token));
    this.lastToken = token;
    this.state = state;
    this.tokens.add(token);
    ++this.scanOffset;
  }

  const syntaxOfToken = (function() {
    const map = new Map();
    map.set(lexers.State.DOT, 'operators');
    map.set(lexers.State.LINE_COMMENT, 'comment');
    map.set(lexers.State.OPERATOR, 'operators');
    map.set(lexers.State.OTHER, '');
    map.set(lexers.State.SPACE, '');
    map.set(lexers.State.STRING1, 'string_literal');
    map.set(lexers.State.STRING1_END, 'string_literal');
    map.set(lexers.State.STRING1_ESCAPE, 'string_literal');
    map.set(lexers.State.STRING2, 'string_literal');
    map.set(lexers.State.STRING2_END, 'string_literal');
    map.set(lexers.State.STRING2_ESCAPE, 'string_literal');
    map.set(lexers.State.STRING3, 'string_literal');
    map.set(lexers.State.STRING3_END, 'string_literal');
    map.set(lexers.State.STRING3_ESCAPE, 'string_literal');
    map.set(lexers.State.WORD, 'identifier');

    /**
     * @param {!lexers.Token} token
     * @return {string}
     */
    function syntaxOfToken(token) {
      if (token.state == lexers.State.WORD) {
        const syntax = this.keywords.get(this.tokenTextOf(token));
        if (syntax)
          return syntax;
      }
      return map.get(token.state) || '';
    }
    return syntaxOfToken;
  })();

  /**
   * @this {!Lexer}
   * @param {number} charCode
   *  Implements common tokens:
   *    - string in single quote with backslash escaping
   *    - string in double quote with backslash escaping
   *    - line comment
   */
  function updateState(charCode) {
    let lexer = this;
    let charSyntax = lexer.characters_.get(charCode);
    switch (lexer.state) {
      case lexers.State.DOT:
        lexer.state = lexers.State.ZERO;
        break;
      case lexers.State.LINE_COMMENT:
        if (charCode === Unicode.LF)
          lexer.endToken();
        else
          lexer.extendToken();
        break;

      case lexers.State.OPERATOR:
        if (charSyntax === Lexer.OPERATOR_CHAR)
          lexer.extendToken();
        else
          lexer.endToken();
        break;

      case lexers.State.OTHER:
        if (this.isOtherChar(charCode))
          lexer.extendToken();
        else
          lexer.endToken();
        break;

      case lexers.State.SPACE:
        if (charCode === Unicode.SPACE || charCode === Unicode.TAB)
          lexer.extendToken();
        else
          lexer.endToken();
        break;

      case lexers.State.STRING1:
        if (charSyntax === Lexer.STRING1_CHAR)
          lexer.finishState(lexers.State.STRING1_END);
        else if (charCode === Unicode.REVERSE_SOLIDUS)
          lexer.finishState(lexers.State.STRING1_ESCAPE);
        else
          lexer.extendToken();
        break;
      case lexers.State.STRING1_END:
        lexer.endToken();
        break;
      case lexers.State.STRING1_ESCAPE:
        lexer.finishState(lexers.State.STRING1);
        break;

      case lexers.State.STRING2:
        if (charSyntax === Lexer.STRING2_CHAR)
          lexer.finishState(lexers.State.STRING2_END);
        else if (charCode === Unicode.REVERSE_SOLIDUS)
          lexer.finishState(lexers.State.STRING2_ESCAPE);
        else
          lexer.extendToken();
        break;
      case lexers.State.STRING2_END:
        lexer.endToken();
        break;
      case lexers.State.STRING2_ESCAPE:
        lexer.finishState(lexers.State.STRING2);
        break;

      case lexers.State.STRING3:
        if (charSyntax === Lexer.STRING3_CHAR)
          lexer.finishState(lexers.State.STRING3_END);
        else if (charCode === Unicode.REVERSE_SOLIDUS)
          lexer.finishState(lexers.State.STRING3_ESCAPE);
        else
          lexer.extendToken();
        break;
      case lexers.State.STRING3_END:
        lexer.endToken();
        break;
      case lexers.State.STRING3_ESCAPE:
        lexer.finishState(lexers.State.STRING3);
        break;

      case lexers.State.WORD:
        if (charSyntax === Lexer.NAMESTART_CHAR ||
            charSyntax === Lexer.NAME_CHAR) {
          lexer.extendToken();
          break;
        }
        lexer.endToken();
        break;

      case lexers.State.ZERO:
        switch (charSyntax) {
          case Lexer.NAMESTART_CHAR:
          case Lexer.NAME_CHAR:
            lexer.startToken(lexers.State.WORD);
            break;
          case Lexer.OPERATOR_CHAR:
            lexer.startToken(lexers.State.OPERATOR);
            break;
          case Lexer.STRING1_CHAR:
            lexer.startToken(lexers.State.STRING1);
            break;
          case Lexer.STRING2_CHAR:
            lexer.startToken(lexers.State.STRING2);
            break;
          case Lexer.STRING3_CHAR:
            lexer.startToken(lexers.State.STRING3);
            break;
          case Lexer.WHITESPACE_CHAR:
            lexer.startToken(lexers.State.SPACE);
            break;
          default:
            lexer.startToken(lexers.State.OTHER);
            break;
        }
        break;

      default:
        console.log(lexer);
        throw new Error(`Invalid state ${lexer.state}`);
    }
  }

  Object.defineProperties(Lexer.prototype, {
    // Properties
    debug_: {value: 0, writable: true},
    parentLexer_: {value: null, writable: true},
    tokenData: {value: null, writable: true},

    // Methods
    adjustScanOffset: {value: adjustScanOffset},
    clear: {value: clear},
    collectTokens: {value: collectTokens},
    colorToken: {value: colorToken},
    detach: {value: detach},
    doColor: {value: doColor},
    endToken: {value: endToken},
    extendToken: {value: extendToken},
    finishState: {value: finishState},
    isNameChar: {value: isNameChar},
    isNameStartChar: {value: isNameStartChar},
    isOtherChar: {value: isOtherChar},
    isWhitespaceChar: {value: isWhitespaceChar},
    isWordChar: {value: isWordChar},
    restartToken: {value: restartToken},
    startToken: {value: startToken},
    syntaxOfToken: {value: syntaxOfToken},
    updateState: {value: updateState},
  });

  global['Lexer'] = Lexer;
})();
