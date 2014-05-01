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
   * @param {!Array.<!MutationRecord>} mutations
   */
  function updateChangedOffset(lexer, mutations) {
    /** @type {number} */
    lexer.changedOffset = mutations.reduce(function(previousValue, mutation) {
      return Math.min(previousValue, mutation.offset);
    }, lexer.changedOffset);
  }

  /**
   * @param {!Iterable.<string>} keywords
   * @param {!Document} document
   */
  function Lexer(keywords, document) {
    this.changedOffset = Count.FORWARD;
    this.count = 0;
    this.debug_ = 0;
    this.lastToken = null;
    this.keywords = makeKeywords(keywords);
    this.mutationObserver_ = new MutationObserver(
        mutationCallback.bind(this));
    this.mutationObserver_.observe(document, {summary: true});
    this.range = new Range(document);
    this.scanOffset = 0;
    this.state = 0;
    this.tokens = new OrderedSet(function(a, b) {
      return a.end < b.end;
    });
  }

  /**
   * @param {!Array.<string>} keywords
   * @return {!Set.<string>}
   */
  // TODO(yosi) Once |Set| constructor accept |Iterable|, we don't need to
  // have |Lexer.makeKeywords|.
  Lexer.makeKeyords_DEPRECATED = function(keywords) {
    var set = new Set();
    keywords.forEach(function(keyword) {
      set.add(keyword);
    });
    return set;
  };

  /**
   * @constructor
   * @param {number} state
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
          console.log('All tokens are dirty');
          this.lastToken = null;
          this.state = 0;
          this.tokens.clear();
          if (oldScanOffset != newScanOffset)
            this.didChangeScanOffset();
          return;
        }
        var dummyToken = new Lexer.Token(0, newScanOffset - 1);
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
          this.state = 0;
          console.log('adjustScanOffset', 'changed at end');
          return;
        }
        var lastToken = it.data;

        // Remove dirty tokens
        if (this.lastToken !== lastToken) {
          if (this.debug_ > 1)
            console.log('change lastToken', newScanOffset, lastToken, 'was', this.lastToken);

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
       * @return number
       */
      function(number) {
        return 0;
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

    finishToken: {value:
      /**
       * @this {!Lexer}
       * @param {number} nextState
       * @return {!Lexer.Token}
       */
      function(nextState) {
        var lastToken = this.lastToken;
        if (!lastToken)
          throw new Error('Assertion failed: lastToken != null');
        if (this.debug_ > 2)
          console.log('finishToken', lastToken);
        if (!nextState)
          this.state = 0;
        else
          this.startToken(nextState);
        return lastToken;
      }
    },

    finishTokenAs: {value:
      /**
       * @this {!Lexer}
       * @param {number} state
       * @return {!Lexer.Token}
       */
      function(state) {
        console.assert(state, 'state must not be zero.');
        var lastToken = this.lastToken;
        this.extendToken();
        if (this.debug_ > 2)
          console.log('finishTokenAs', state, lastToken);
        this.state = state;
        lastToken.state = state;
        return lastToken;
      }
    },

    startToken: {value:
      /**
       * @this {!Lexer}
       * @param {number} state
       */
      function(state) {
        console.assert(state, 'state must not be zero.');
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
   }
  });

  return Lexer;
})();
