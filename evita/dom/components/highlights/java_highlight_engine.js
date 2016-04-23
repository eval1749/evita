// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights.base');

goog.scope(function() {

const JavaTokenStateMachine = highlights.JavaTokenStateMachine;
const HighlightEngine = highlights.base.HighlightEngine;
const KeywordPainter = highlights.base.KeywordPainter;
const Token = highlights.base.Token;
const Tokenizer = highlights.base.Tokenizer;

/** @const @type {!Set<string>} */
const staticJavaKeywords = highlights.staticJavaKeywords;

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isWhitespace(charCode) {
  return charCode <= Unicode.SPACE;
}

class JavaPainter extends KeywordPainter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document, staticJavaKeywords, '.'); }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!JavaPainter}
   */
  static create(document) { return new JavaPainter(document); }
}

class JavaHighlightEngine extends HighlightEngine {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document, JavaPainter.create, new JavaTokenStateMachine());
  }

  /**
   * @public
   * @return {!Set<string>}
   * For debugging
   */
  static get keywords() { return staticJavaKeywords; }

  /**
   * @public
   * @param {string} word
   * Adds a keyword at runtime.
   */
  static addKeyword(word) { staticJavaKeywords.add(word); }
}

/** @constructor */
highlights.JavaHighlightEngine = JavaHighlightEngine;
// Export |JavaPainter| for testing.
/** @constructor */
highlights.JavaPainter = JavaPainter;
});

// Override |JavaLexer| by |JavaHighlightEngine|.
// TODO(eval1749): Once we get rid of |JavaLexer|, we should get rid of
// this
// override.
global['JavaLexer'] = highlights.JavaHighlightEngine;
