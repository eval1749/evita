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
    this.lastToken = null;
    this.scanOffset = 0;
    this.state = Lexer.State.ZERO;
    this.tokens.clear();
    this.doColor(this.range.document.length);
  }

  /**
   * @this {!Lexer}
   */
  function willLoadDocument() {
    this.mutationObserver_.disconnect();
  }

  /**
   * @param {!Iterable.<string>} keywords
   * @return {!Set.<string>}
   */
  function makeKeywords(keywords) {
    var keywordSet = new Set();
    keywords.forEach(function(keyword) {
      keywordSet.add(keyword);
    });
    return keywordSet;
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
    this.lastToken = null;
    this.keywords = makeKeywords(options.keywords);
    this.mutationObserver_ = new MutationObserver(
        mutationCallback.bind(this));
    this.range = new Range(document);
    this.scanOffset = 0;
    this.state = Lexer.State.ZERO;
    this.stateToSyntax_ = options.stateToSyntax;
    this.tokens = new OrderedSet(function(a, b) {
      return a.end < b.end;
    });

    this.eventHandlers_ = new Map();
    function installEventHandler(eventType, lexer, callback) {
      var handler = callback.bind(lexer);
      lexer.eventHandlers_.set(eventType, handler);
      document.addEventListener(eventType, handler);
    }
    installEventHandler(Event.Names.BEFORELOAD, this, willLoadDocument);
    installEventHandler(Event.Names.LOAD, this, didLoadDocument);
    setupMutationObserver(this);
  }

  Lexer.OPERATOR_CHAR = Symbol('operator');
  Lexer.OTHER_CHAR = Symbol('other');
  Lexer.WHITESPACE_CHAR = Symbol('whitespace');
  Lexer.STRING1_CHAR = Symbol('string1');
  Lexer.STRING2_CHAR = Symbol('string2');
  Lexer.WORD_CHAR = Symbol('word');

  Lexer.State = {};
  Lexer.State.LINE_COMMENT = Symbol('line_comment');
  Lexer.State.OPERATOR = Symbol('operator');
  Lexer.State.OTHER = Symbol('other');
  Lexer.State.SPACE = Symbol('space');
  Lexer.State.STRING1 = Symbol('string1');
  Lexer.State.STRING1_END = Symbol('string1_end');
  Lexer.State.STRING1_ESCAPE = Symbol('string1_escape');
  Lexer.State.STRING2 = Symbol('string2');
  Lexer.State.STRING2_END = Symbol('string2_end');
  Lexer.State.STRING2_ESCAPE = Symbol('string2_escape');
  Lexer.State.ZERO = Symbol('zero');

  var stateToSyntax = new Map();
  stateToSyntax.set(Lexer.State.LINE_COMMENT, 'comment');
  stateToSyntax.set(Lexer.State.OPERATOR, 'operators');
  stateToSyntax.set(Lexer.State.OTHER, '');
  stateToSyntax.set(Lexer.State.SPACE, '');
  stateToSyntax.set(Lexer.State.STRING1, 'string_literal');
  stateToSyntax.set(Lexer.State.STRING1_END, 'string_literal');
  stateToSyntax.set(Lexer.State.STRING1_ESCAPE, 'string_literal');
  stateToSyntax.set(Lexer.State.STRING2, 'string_literal');
  stateToSyntax.set(Lexer.State.STRING2_END, 'string_literal');
  stateToSyntax.set(Lexer.State.STRING2_ESCAPE, 'string_literal');
  stateToSyntax.set(Lexer.State.WORD, 'identifier');

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

  Object.defineProperties(Lexer.prototype, {
    adjustScanOffset: {value:
      /**
       * @this {!Lexer}
       */
      function() {
        if (!this.lastToken) {
          console.assert(!this.tokens.size);
          return;
        }
        var document = this.range.document;
        var oldScanOffset = this.scanOffset;
        var newScanOffset = Math.min(this.changedOffset, this.lastToken.end,
                                     document.length);
        this.scanOffset = newScanOffset;
        this.changedOffset = Count.FORWARD;
        if (!newScanOffset) {
          // All tokens in token list are dirty.
          if (this.debug_ > 0)
            console.log('All tokens are dirty');
          this.lastToken = null;
          this.state = Lexer.State.ZERO;
          this.tokens.clear();
          if (oldScanOffset != newScanOffset)
            this.didChangeScanOffset();
          return;
        }
        var dummyToken = new Lexer.Token(Lexer.State.ZERO, newScanOffset - 1);
        var it = this.tokens.lowerBound(dummyToken);
        // TODO(yosi) We should use |OrderedSet.prototype.upperBound()|
        if (it && it.data.end == newScanOffset - 1)
          it = it.next();

        // Case 1: <ss|ss> middle of token
        // Case 2: <|ssss> start of token
        //
        if (!it) {
          // Document mutation is occurred after scanned range. We continue
          // scanning from current position;
          this.state = Lexer.State.ZERO;
          if (this.debug_ > 0)
            console.log('adjustScanOffset', 'changed at end');
          return;
        }
        var lastToken = it.data;

        // Remove dirty tokens
        if (this.lastToken !== lastToken) {
          if (this.debug_ > 1) {
            console.log('change lastToken', newScanOffset, lastToken, 'was',
                        this.lastToken);
          }
          this.lastToken = lastToken;
          this.state = lastToken.state;

          // Collect dirty tokens
          var tokensToRemove = new Array();
          for (it = it.next(); it; it = it.next()) {
            tokensToRemove.push(it.data);
          }
          if (this.debug_ > 4)
            console.log('tokensToRemove', tokensToRemove);
          // Remove dirty tokens from set.
          tokensToRemove.forEach(function(token) {
            this.tokens.remove(token);
          }, this);
        }

        // Shrink last clean token
        if (lastToken.end != newScanOffset) {
          lastToken.end = newScanOffset;
          this.didShrinkLastToken(lastToken);
          this.state = lastToken.state;
        }
        if (oldScanOffset != newScanOffset)
          this.didChangeScanOffset();
      }
    },

    debug_: {value: 0, writable: true},

    detach: {value:
      /**
       * @this {!Lexer}
       */
      function() {
        if (!this.range)
          throw new Error(this + ' isn\'t attached to document.');
        var document = this.range.document;
        this.eventHandlers_.forEach(function(handler, eventType) {
            document.removeEventListener(eventType, handler);
        });
        this.mutationObserver_.disconnect();
        this.range = null;
      }
    },

    didChangeScanOffset: {value:
      /**
       * @this {!Lexer}
       */
      function() {
        // nothing to do
      }
    },

    didShrinkLastToken: {value:
      /**
       * @this {!Lexer}
       * @param {!Lexer.Token} token
       */
      function(token) {
        // nothing to do
      }
    },

    doColor: {value:
      /**
       * @this {!Lexer}
       * @param {number} maxCount
       * @return number
       */
      function(maxCount) {
        if (!this.range)
          throw new Error("Can't use disconnected lexer.");

        this.adjustScanOffset();
        var document = this.range.document;
        var maxOffset = Math.min(this.scanOffset + maxCount, document.length);
        var startOffset = this.scanOffset;
        while (this.scanOffset < maxOffset) {
          var token = this.nextToken(maxOffset);
          if (!token)
            break;
          this.setSyntax(token);
          if (this.lastToken != token)
            this.setSyntax(/** @type {!Lexer.Token} */(this.lastToken));
        }
        return maxCount - (this.scanOffset - startOffset);
      }
    },

    extendToken: {value:
      /**
       * @this {!Lexer}
       */
      function() {
        ++this.scanOffset;
        this.lastToken.end = this.scanOffset;
      },
    },

    extractWord: {value:
      /**
       * @this {!Lexer}
       * @param {!Range} range
       * @param {!Lexer.Token} token
       * @return {string}
       */
      function(range, token) {
        return range.text;
      }
    },

    finishToken: {value:
      /**
       * @this {!Lexer}
       * @param {!Lexer.State} nextState
       * @return {!Lexer.Token}
       */
      function(nextState) {
        var lastToken = this.lastToken;
        if (!lastToken)
          throw new Error('Assertion failed: lastToken != null');
        if (this.debug_ > 2)
          console.log('finishToken', lastToken);
        if (nextState == Lexer.State.ZERO)
          this.state = nextState;
        else
          this.startToken(nextState);
        return lastToken;
      }
    },

    finishTokenAs: {value:
      /**
       * @this {!Lexer}
       * @param {!Lexer.State} state
       * @return {!Lexer.Token}
       */
      function(state) {
        console.assert(state != Lexer.State.ZERO, 'state must not be zero.');
        var lastToken = this.lastToken;
        this.extendToken();
        if (this.debug_ > 2)
          console.log('finishTokenAs', state, lastToken);
        this.state = state;
        lastToken.state = state;
        return lastToken;
      }
    },

    isOperator: {value:
      /**
       * @this {!Lexer}
       * @param {number} charCode
       * @return {boolean}
       */
      function(charCode) {
        return this.characters_.get(charCode) == Lexer.OPERATOR_CHAR;
      }
    },

    isOther: {value:
      /**
       * @this {!Lexer}
       * @param {number} charCode
       * @return {boolean}
       */
      function(charCode) {
        return !this.characters_.has(charCode);
      }
    },

    isWhitespace: {value:
      /**
       * @this {!Lexer}
       * @param {number} charCode
       * @return {boolean}
       */
      function(charCode) {
        return this.characters_.get(charCode) == Lexer.WHITESPACE_CHAR;
      }
    },

    isWord: {value:
      /**
       * @this {!Lexer}
       * @param {number} charCode
       * @return {boolean}
       */
      function(charCode) {
        return this.characters_.get(charCode) == Lexer.WORD_CHAR;
      }
    },

    setSyntax: {value:
      /**
       * @this {!Lexer}
       * @param {!Lexer.Token} token
       */
      function setSyntax(token) {
        var range = this.range;
        range.collapseTo(token.start);
        range.end = token.end;
        var syntax = this.stateToSyntax_.get(token.state) ||
                     stateToSyntax.get(token.state) || '';
        if (syntax == 'identifier') {
          var word = this.extractWord(range, token);
          if (this.debug_ > 5)
            console.log('setSyntax', '"' + word + '"');
          syntax = this.keywords.has(word) ? 'keyword' : '';
        }
        if (this.debug_ > 4)
          console.log('setSyntax', syntax, token);
        range.setSyntax(syntax);
      }
    },

    startToken: {value:
      /**
       * @this {!Lexer}
       * @param {!Lexer.State} state
       */
      function(state) {
        console.assert(state != Lexer.State.ZERO, 'state must not be zero.');
        this.range.collapseTo(this.scanOffset);
        if (this.debug_ > 2)
          console.log('startToken', state, this.scanOffset);
        var token = new Lexer.Token(state, this.scanOffset);
        if (this.debug_ > 0)
          console.assert(!this.tokens.find(token));
        this.lastToken = token;
        this.state = state;
        this.tokens.add(token);
        ++this.scanOffset;
      }
   },

   updateState: {value:
     /**
      * @protected
      * @this {!Lexer}
      * @param {number} charCode
      * @return {?Lexer.Token}
      */
     function(charCode) {
       var lexer = this;
       switch (lexer.state){
        case Lexer.State.LINE_COMMENT:
          if (charCode == Unicode.LF)
            return lexer.finishToken(Lexer.State.ZERO);
          lexer.extendToken();
          break;

        case Lexer.State.OPERATOR:
          if (!lexer.isOperator(charCode))
            return lexer.finishToken(Lexer.State.ZERO);
          lexer.extendToken();
          break;

        case Lexer.State.OTHER:
          if (!this.isOther(charCode))
            return lexer.finishToken(Lexer.State.ZERO);
          lexer.extendToken();
          break;

        case Lexer.State.SPACE:
          if (charCode != Unicode.SPACE && charCode != Unicode.TAB)
            return lexer.finishToken(Lexer.State.ZERO);
          lexer.extendToken();
          break;

        case Lexer.State.STRING1:
          if (charCode == Unicode.APOSTROPHE)
            return lexer.finishToken(Lexer.State.STRING2_END);
          if (charCode == Unicode.REVERSE_SOLIDUS)
            return lexer.finishToken(Lexer.State.STRING1_ESCAPE);
          lexer.extendToken();
          break;
        case Lexer.State.STRING1_END:
          return lexer.finishToken(Lexer.State.ZERO);
        case Lexer.State.STRING1_ESCAPE:
          return lexer.finishToken(Lexer.State.STRING1);

        case Lexer.State.STRING2:
          if (charCode == Unicode.QUOTATION_MARK)
            return lexer.finishToken(Lexer.State.STRING2_END);
          if (charCode == Unicode.REVERSE_SOLIDUS)
            return lexer.finishToken(Lexer.State.STRING2_ESCAPE);
          lexer.extendToken();
          break;
        case Lexer.State.STRING2_END:
          return lexer.finishToken(Lexer.State.ZERO);
        case Lexer.State.STRING2_ESCAPE:
          return lexer.finishToken(Lexer.State.STRING2);

        case Lexer.State.WORD:
          if (!lexer.isWord(charCode))
            return lexer.finishToken(Lexer.State.ZERO);
          lexer.extendToken();
          break;

        case Lexer.State.ZERO:
          switch (charCode) {
            case Unicode.APOSTROPHE:
              lexer.startToken(Lexer.State.STRING1);
              break;
            case Unicode.LF:
            case Unicode.SPACE:
            case Unicode.TAB:
              lexer.startToken(Lexer.State.SPACE);
              break;
            case Unicode.QUOTATION_MARK:
              lexer.startToken(Lexer.State.STRING2);
              break;
            default:
              if (lexer.isWord(charCode))
                lexer.startToken(Lexer.State.WORD);
              else if (lexer.isOperator(charCode))
                lexer.startToken(Lexer.State.OPERATOR);
              else
                lexer.startToken(Lexer.State.OTHER);
              break;
          }
          break;

        default:
          console.log(lexer);
          throw new Error('Invalid state ' + lexer.state);
       }
       return null;
     }
   },
  });

  return Lexer;
})();
