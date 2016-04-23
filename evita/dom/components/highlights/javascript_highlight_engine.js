// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights.base');

goog.scope(function() {

const JavascriptTokenStateMachine = highlights.JavascriptTokenStateMachine;
const HighlightEngine = highlights.base.HighlightEngine;
const KeywordPainter = highlights.base.KeywordPainter;
const Token = highlights.base.Token;
const Tokenizer = highlights.base.Tokenizer;

/** @const @type {!Set<string>} */
const staticJavascriptKeywords = highlights.staticJavascriptKeywords;

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isWhitespace(charCode) {
  return charCode <= Unicode.SPACE;
}

class JavascriptPainter extends KeywordPainter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document, staticJavascriptKeywords, '.'); }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!JavascriptPainter}
   */
  static create(document) { return new JavascriptPainter(document); }
}

class JavascriptHighlightEngine extends HighlightEngine {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(
        document, JavascriptPainter.create, new JavascriptTokenStateMachine());
  }

  /**
   * @public
   * @return {!Set<string>}
   * For debugging
   */
  static get keywords() { return staticJavascriptKeywords; }

  /**
   * @public
   * @param {string} word
   * Adds a keyword at runtime.
   */
  static addKeyword(word) { staticJavascriptKeywords.add(word); }
}

/** @constructor */
highlights.JavascriptHighlightEngine = JavascriptHighlightEngine;
// Export |JavascriptPainter| for testing.
/** @constructor */
highlights.JavascriptPainter = JavascriptPainter;
});

// Override |JavascriptLexer| by |JavascriptHighlightEngine|.
// TODO(eval1749): Once we get rid of |JavascriptLexer|, we should get rid of
// this
// override.
global['JavascriptLexer'] = highlights.JavascriptHighlightEngine;
