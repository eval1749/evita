// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');

goog.scope(function() {

const KeywordPainter = highlights.KeywordPainter;

/** @const @type {!Set<string>} */
const staticJavaScriptKeywords =
    highlights.HighlightEngine.keywordsFor('javascript');

class JavaScriptPainter extends KeywordPainter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document, staticJavaScriptKeywords, '.'); }

  /**
   * @override
   */
  paint(token) {
    if (token.syntax !== 'identifier' || token.length <= 2)
      return super.paint(token);
    if (this.document.charCodeAt(token.end - 1) !== Unicode.COLON)
      return super.paint(token);
    const syntax =
        staticJavaScriptKeywords.has(this.textOf(token)) ? 'keyword' : 'label';
    this.setSyntax(token.start, token.end - 1, syntax);
    this.setSyntax(token.end - 1, token.end, 'operator');
  }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!JavaScriptPainter}
   */
  static create(document) { return new JavaScriptPainter(document); }
}

// Export |JavaScriptPainter| for testing.
/** @constructor */
highlights.JavaScriptPainter = JavaScriptPainter;
});
