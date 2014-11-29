// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 *
 * @param {!Document} document
 *
 * Note: Closure compiler doesn't allow to write |function Lexer|, we use
 * IIFE to set constructor name to |Lexer| rather than |global.Lexer|.
 */
global.Lexer = (function() {
  /**
   * @this {!Lexer}
   */
  function didLoadDocument() {
    setupMutationObserver(this);
    this.clear();
    this.doColor(this.range.document.length);
  }

  /**
   * @this {!Lexer}
   */
  function willLoadDocument() {
    this.mutationObserver_.disconnect();
  }

  /**
   * @this {!Lexer}
   * @param {!Array.<!MutationRecord>} mutations
   * @param {!MutationObserver} observer
   *
   * Resets |changedOffset| to minimal changed offset.
   */
  function mutationCallback(mutations, observer) {
    updateChangedOffset(this, mutations);
  }

  /**
   * @param {!Lexer} lexer
   */
  function setupMutationObserver(lexer) {
    lexer.mutationObserver_.observe(lexer.range.document, {summary: true});
  }

  /**
   * @param {!Lexer} lexer
   * @param {!Array.<!MutationRecord>} mutations
   */
  function updateChangedOffset(lexer, mutations) {
    /** @type {number} */
    lexer.changedOffset = mutations.reduce(function(previousValue, mutation) {
      return Math.min(previousValue, mutation.offset);
    }, lexer.changedOffset);
  }

  /**
   * @param {!Document} document
   * @param {!LexerOptions} options
   */
  function Lexer(document, options) {
    this.characters_ = options.characters;
    this.changedOffset = Count.FORWARD;
    this.debug_ = 0;
    this.keywords = options.keywords;
    this.lastToken = null;
    // TODO(yosi) |maxChainWords_| should be part of |LexerOptions|.
    this.maxChainWords_ = 3;
    this.range = new Range(document);
    this.scanOffset = 0;
    this.state = Lexer.State.ZERO;
    this.tokens = new OrderedSet(function(a, b) {
      return a.end < b.end;
    });

    if (this.parentLexer_)
      return;

    this.mutationObserver_ = new MutationObserver(
        mutationCallback.bind(this));
    this.eventHandlers_ = new Map();
    function installEventHandler(eventType, lexer, callback) {
      let handler = callback.bind(lexer);
      lexer.eventHandlers_.set(eventType, handler);
      document.addEventListener(eventType, handler);
    }
    installEventHandler(Event.Names.BEFORELOAD, this, willLoadDocument);
    installEventHandler(Event.Names.LOAD, this, didLoadDocument);
    setupMutationObserver(this);
  }

  Lexer.NAME_CHAR = Symbol('name');
  Lexer.NAMESTART_CHAR = Symbol('nameStart');
  Lexer.OPERATOR_CHAR = Symbol('operator');
  Lexer.OTHER_CHAR = Symbol('other');
  Lexer.WHITESPACE_CHAR = Symbol('whitespace');
  Lexer.STRING1_CHAR = Symbol('string1');
  Lexer.STRING2_CHAR = Symbol('string2');
  Lexer.STRING3_CHAR = Symbol('string3');

  Lexer.State = {
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

  /**
   * @constructor
   * @param {!Lexer.State} state
   * @param {number} start
   */
  Lexer.Token = (function() {
    function Token(state, start) {
      this.end = start + 1;
      this.start = start;
      this.state = state;
    }
    return Token;
  })();

  /**
   * @param {!Array.<string>} keywords
   * @return {!Map.<string, string>}
   */
  Lexer.createKeywords = function(keywords) {
    let map = new Map();
    keywords.forEach(function(keyword) {
      map.set(keyword, 'keyword');
    });
    return map;
  }

  /**
   * @this {!Lexer}
   */
  function adjustScanOffset() {
    if (!this.lastToken || this.changedOffset === Count.FORWARD)
      return;
    let document = this.range.document;
    let newScanOffset = Math.min(this.changedOffset, this.lastToken.end,
                                 document.length);
    if (this.debug_ > 0)
      console.log('adjustScanOffset', newScanOffset, this);

    this.scanOffset = newScanOffset;
    this.changedOffset = Count.FORWARD;
    if (!newScanOffset || newScanOffset <= this.tokens.minimum.start) {
      // All tokens in token list are dirty.
      this.clear();
      return;
    }

    let dummyToken = new Lexer.Token(Lexer.State.ZERO, newScanOffset - 1);
    let it = this.tokens.lowerBound(dummyToken);
    console.assert(it, newScanOffset);

    if (this.debug_ > 1)
      console.log('restart from', it.data);

    // Case 1: <ss|ss> middle of token
    // Case 2: ssss| end of token
    let lastToken = it.data;
    it = it.next();

    // Remove dirty tokens
    if (it) {
      // Collect dirty tokens
      let tokensToRemove = new Array();
      while (it) {
        tokensToRemove.push(it.data);
        it = it.next();
      }
      if (this.debug_ > 4)
        console.log('tokensToRemove', tokensToRemove);
      // Remove dirty tokens from set.
      tokensToRemove.forEach(function(token) {
        this.tokens.remove(token);
      }, this);
    }

    // Shrink last clean token
    if (lastToken.end !== newScanOffset) {
      console.assert(lastToken.start < newScanOffset, lastToken);
      lastToken.end = newScanOffset;
      let newState = this.didShrinkLastToken(lastToken);
      if (lastToken.state !== newState)
        lastToken.state = newState;
    }

    this.lastToken = lastToken;
    this.state = lastToken.state;
    this.tokenData = lastToken.data;
    this.colorToken(lastToken);
  }

  /**
   * @this {!Lexer}
   */
  function clear() {
    if (this.debug_ > 0)
      console.log('Lexer.clear', this);
    this.lastToken = null;
    this.scanOffset = 0;
    this.state = Lexer.State.ZERO;
    this.tokens.clear();
  }

  /**
   * @this {!Lexer}
   * @param {!OrderedSetNode.<!Lexer.Token>} itDelimiter
   * @param {!Lexer.Token} token
   * @return {!Array.<!Lexer.Token>}
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
      } while (it && it.data.state === Lexer.State.SPACE);
      if (!it || it.data.state !== Lexer.State.WORD)
        break;
      tokens.unshift(it.data);
      it = it.previous();
    }
    return tokens;
  }

  /**
   * @this {!Lexer}
   */
  function colorLastToken() {
    let token = this.lastToken;
    if (!token)
      return;
    this.colorToken(token);
  }

  /**
   * @this {!Lexer}
   * @param {!Lexer.Token} token
   */
  function colorToken(token) {
    let range = this.range;
    range.collapseTo(token.start);
    range.end = token.end;
    let syntax = this.syntaxOfToken(range, token) || '';
    if (this.debug_ > 4)
      console.log('setSyntax', '"' + syntax + '"', token);
    range.setSyntax(syntax === 'identifier' ? '' : syntax);
  }

  /**
   * @this {!Lexer}
   */
  function detach() {
    if (!this.range)
      throw new Error(this + ' isn\'t attached to document.');
    let document = this.range.document;
    this.eventHandlers_.forEach(function(handler, eventType) {
        document.removeEventListener(eventType, handler);
    });
    this.mutationObserver_.disconnect();
    this.range = null;
  }

  /**
   * @this {!Lexer}
   * @param {!Lexer.Token} token
   */
  function didShrinkLastToken(token) {
    // nothing to do
  }

  /**
   * @this {!Lexer}
   * @param {number} maxCount
   * @return number
   */
  function doColor(maxCount) {
    if (!this.range)
      throw new Error("Can't use disconnected lexer.");

    this.adjustScanOffset();
    let document = this.range.document;
    let maxOffset = Math.min(this.scanOffset + maxCount, document.length);
    let startOffset = this.scanOffset;
    while (this.scanOffset < maxOffset) {
      let charCode = document.charCodeAt_(this.scanOffset);
      this.feedCharacter(charCode);
    }
    let count = this.scanOffset - startOffset;
    if (count && this.lastToken)
      this.colorLastToken();
    return maxCount - count;
  }

  /**
   * @this {!Lexer}
   */
  function endToken() {
    this.colorLastToken();
    this.state = Lexer.State.ZERO;
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
   * @param {!Lexer.State} nextState
   */
  function finishToken(nextState) {
    if (this.debug_ > 2)
      console.log('finishToken', this.lastToken);
    this.colorLastToken();
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
  function isOtherChar(charCode) {
    return !this.characters_.has(charCode);
  }

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
   * @param {!Lexer.State} newState
   */
  function restartToken(newState) {
    console.assert(newState !== Lexer.State.ZERO,
                   'newState must not be zero.');
    this.extendToken();
    if (this.debug_ > 2)
      console.log('restartToken', newState, this.lastToken);
    this.state = newState;
    this.lastToken.state = newState;
    this.colorLastToken();
  }

  /**
   * @this {!Lexer}
   * @param {!Lexer.State} state
   */
  function startToken(state) {
    console.assert(state !== Lexer.State.ZERO, 'state must not be zero.');
    if (this.debug_ > 2)
      console.log('startToken', state, this.scanOffset);
    let token = new Lexer.Token(state, this.scanOffset);
    token.data = this.tokenData;
    if (this.debug_ > 0)
      console.assert(!this.tokens.find(token));
    this.lastToken = token;
    this.state = state;
    this.tokens.add(token);
    ++this.scanOffset;
  }

  let syntaxOfToken = (function() {
    let map = new Map();
    map.set(Lexer.State.DOT, 'operators');
    map.set(Lexer.State.LINE_COMMENT, 'comment');
    map.set(Lexer.State.OPERATOR, 'operators');
    map.set(Lexer.State.OTHER, '');
    map.set(Lexer.State.SPACE, '');
    map.set(Lexer.State.STRING1, 'string_literal');
    map.set(Lexer.State.STRING1_END, 'string_literal');
    map.set(Lexer.State.STRING1_ESCAPE, 'string_literal');
    map.set(Lexer.State.STRING2, 'string_literal');
    map.set(Lexer.State.STRING2_END, 'string_literal');
    map.set(Lexer.State.STRING2_ESCAPE, 'string_literal');
    map.set(Lexer.State.STRING3, 'string_literal');
    map.set(Lexer.State.STRING3_END, 'string_literal');
    map.set(Lexer.State.STRING3_ESCAPE, 'string_literal');
    map.set(Lexer.State.WORD, 'identifier');

    /**
     * @param {!Lexer.Token} token
     * @param {!Range} range
     * @return {string}
     */
    return function(range, token) {
      return map.get(token.state);
    };
  })();

  /**
   * @this {!Lexer} lexer
   * @param {!Range} range
   * @param {!Array.<!Lexer.Token>} tokens
   * @return {string}
   */
  function syntaxOfTokens(range, tokens) {
    let lexer = this;
    let document = range.document;
    let words = tokens.map(function(token) {
      return document.slice(token.start, token.end);
    });
    while (words.length) {
      let syntax = lexer.keywords.get(words.join(''));
      if (syntax) {
        range.start = tokens[0].start;
        return syntax;
      }
      words.shift();
      tokens.shift();
    }
    return '';
  }

  /**
   * @this {!Lexer}
   * @param {string} word
   * @return {string}
   */
  function syntaxOfWord(word) {
    return this.keywords.has(word) ? 'keyword' : 'identifier';
  }

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
    switch (lexer.state){
     case Lexer.State.DOT:
       lexer.state = Lexer.State.ZERO;
       break;
     case Lexer.State.LINE_COMMENT:
       if (charCode === Unicode.LF)
         lexer.endToken();
       else
         lexer.extendToken();
       break;

     case Lexer.State.OPERATOR:
       if (charSyntax === Lexer.OPERATOR_CHAR)
         lexer.extendToken();
       else
         lexer.endToken();
       break;

     case Lexer.State.OTHER:
       if (this.isOtherChar(charCode))
         lexer.extendToken();
       else
         lexer.endToken();
       break;

     case Lexer.State.SPACE:
       if (charCode === Unicode.SPACE || charCode === Unicode.TAB)
         lexer.extendToken();
       else
         lexer.endToken();
       break;

     case Lexer.State.STRING1:
       if (charSyntax === Lexer.STRING1_CHAR)
         lexer.finishToken(Lexer.State.STRING1_END);
       else if (charCode === Unicode.REVERSE_SOLIDUS)
         lexer.finishToken(Lexer.State.STRING1_ESCAPE);
       else
         lexer.extendToken();
       break;
     case Lexer.State.STRING1_END:
       lexer.endToken();
       break;
     case Lexer.State.STRING1_ESCAPE:
       lexer.finishToken(Lexer.State.STRING1);
       break;

     case Lexer.State.STRING2:
       if (charSyntax === Lexer.STRING2_CHAR)
         lexer.finishToken(Lexer.State.STRING2_END);
       else if (charCode === Unicode.REVERSE_SOLIDUS)
         lexer.finishToken(Lexer.State.STRING2_ESCAPE);
       else
         lexer.extendToken();
       break;
     case Lexer.State.STRING2_END:
       lexer.endToken();
       break;
     case Lexer.State.STRING2_ESCAPE:
       lexer.finishToken(Lexer.State.STRING2);
       break;

     case Lexer.State.STRING3:
       if (charSyntax === Lexer.STRING3_CHAR)
         lexer.finishToken(Lexer.State.STRING3_END);
       else if (charCode === Unicode.REVERSE_SOLIDUS)
         lexer.finishToken(Lexer.State.STRING3_ESCAPE);
       else
         lexer.extendToken();
       break;
     case Lexer.State.STRING3_END:
       lexer.endToken();
       break;
     case Lexer.State.STRING3_ESCAPE:
       lexer.finishToken(Lexer.State.STRING3);
       break;

     case Lexer.State.WORD:
       if (charSyntax === Lexer.NAMESTART_CHAR ||
           charSyntax === Lexer.NAME_CHAR) {
         lexer.extendToken();
         break;
       }
       lexer.endToken();
       break;

     case Lexer.State.ZERO:
       switch (charSyntax) {
         case Lexer.NAMESTART_CHAR:
         case Lexer.NAME_CHAR:
           lexer.startToken(Lexer.State.WORD);
           break;
         case Lexer.OPERATOR_CHAR:
           lexer.startToken(Lexer.State.OPERATOR);
           break;
         case Lexer.STRING1_CHAR:
           lexer.startToken(Lexer.State.STRING1);
           break;
         case Lexer.STRING2_CHAR:
           lexer.startToken(Lexer.State.STRING2);
           break;
         case Lexer.STRING3_CHAR:
           lexer.startToken(Lexer.State.STRING3);
           break;
         case Lexer.WHITESPACE_CHAR:
           lexer.startToken(Lexer.State.SPACE);
           break;
         default:
           lexer.startToken(Lexer.State.OTHER);
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
    colorLastToken: {value: colorLastToken},
    colorToken: {value: colorToken},
    detach: {value: detach},
    didShrinkLastToken: {value: didShrinkLastToken},
    doColor: {value: doColor},
    endToken: {value: endToken},
    extendToken: {value: extendToken},
    finishToken: {value: finishToken},
    isNameChar: {value: isNameChar},
    isNameStartChar: {value: isNameStartChar},
    isOtherChar: {value: isOtherChar},
    isWhitespaceChar: {value: isWhitespaceChar},
    isWordChar: {value: isWordChar},
    restartToken: {value: restartToken},
    startToken: {value: startToken},
    syntaxOfToken: {value: syntaxOfToken},
    syntaxOfTokens: {value: syntaxOfTokens},
    syntaxOfWord: {value: syntaxOfWord},
    updateState: {value: updateState},
  });

  return Lexer;
})();
