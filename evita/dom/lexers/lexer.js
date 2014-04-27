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
   * @param {!Document} document
   */
  function Lexer(document) {
    this.changedOffset = Count.FORWARD;
    this.count = 0;
    this.mutationObserver_ = new MutationObserver(
        mutationCallback.bind(this));
    this.mutationObserver_.observe(document, {summary: true});
    this.range = new Range(document);
    this.scanOffset = 0;
    this.state = 0;
    this.syntax = '';
  }

  Object.defineProperties(Lexer.prototype, {
    adjustScanOffset: {value:
      /**
       * @this {!Lexer}
       * @param {number} maxCount
       * @return {number} Return number of characters scanned.
       */
      function(maxCount) {
        updateChangedOffset(this, this.mutationObserver_.takeRecords());
        var document = this.range.document;
        var startOffset = Math.min(this.changedOffset, document.length);
        this.changedOffset = Count.FORWARD;
        this.count = maxCount;
        if (this.scanOffset <= startOffset) {
          // Modification is occurred after |this.scanOffset|, we can continue
          // scanning with current state.
          return 0;
        }
        // Move backward until start of token.
        var offset = startOffset;
        var syntax = offset < document.length ? document.syntaxAt(offset) : '';
        // Find last token.
        while (offset && !syntax) {
          --offset;
          syntax = document.syntaxAt(offset);
        }
        // Move start of last token.
        while (offset) {
          var syntax2 = document.syntaxAt(offset - 1);
          if (syntax != syntax2)
            break;
          --offset;
        }
        if (this.debug > 2)
          console.log('adjustScanOffset', 'offset', offset);
        this.state = 0;
        this.scanOffset = offset;
        this.syntax = '';
        this.didChangeScanOffset();
        return startOffset - offset;
      }
    },

    debug: {value: 0, writable: true},

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
        this.state = 0;
        this.syntax = '';
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

    finishToken: {value:
      /**
       * @this {!Lexer}
       * @return string
       */
      function() {
        this.range.end = this.scanOffset;
        if (this.debug > 2)
          console.log('finishToken', this.state, this.syntax, this.range);
        var syntax = this.syntax;
        this.state = 0;
        this.syntax = '';
        return syntax;
      }
    },

    startToken: {value:
      /**
       * @this {!Lexer}
       * @param {number} state
       * @param {string} syntax
       */
      function(state, syntax) {
        this.range.collapseTo(this.scanOffset - 1);
        if (this.debug > 2)
          console.log('startToken', state, syntax, this.range);
        this.state = state;
        this.syntax = syntax;
      }
   }
  });

  return Lexer;
})();
