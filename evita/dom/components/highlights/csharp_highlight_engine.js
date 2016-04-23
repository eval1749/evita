// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights.base');

goog.scope(function() {

const CsharpTokenStateMachine = highlights.CsharpTokenStateMachine;
const HighlightEngine = highlights.base.HighlightEngine;
const KeywordPainter = highlights.base.KeywordPainter;
const Token = highlights.base.Token;
const Tokenizer = highlights.base.Tokenizer;

/** @const @type {!Set<string>} */
const staticCsharpKeywords = highlights.staticCsharpKeywords;

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isWhitespace(charCode) {
  return charCode <= Unicode.SPACE;
}

class CsharpPainter extends KeywordPainter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document, staticCsharpKeywords, '.'); }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!CsharpPainter}
   */
  static create(document) { return new CsharpPainter(document); }
}

class CsharpHighlightEngine extends HighlightEngine {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document, CsharpPainter.create, new CsharpTokenStateMachine());
  }

  /**
   * @public
   * @return {!Set<string>}
   * For debugging
   */
  static get keywords() { return staticCsharpKeywords; }

  /**
   * @public
   * @param {string} word
   * Adds a keyword at runtime.
   */
  static addKeyword(word) { staticCsharpKeywords.add(word); }
}

/** @constructor */
highlights.CsharpHighlightEngine = CsharpHighlightEngine;
// Export |CsharpPainter| for testing.
/** @constructor */
highlights.CsharpPainter = CsharpPainter;
});

// Override |CsharpLexer| by |CsharpHighlightEngine|.
// TODO(eval1749): Once we get rid of |CsharpLexer|, we should get rid of
// this
// override.
global['CsharpLexer'] = highlights.CsharpHighlightEngine;
