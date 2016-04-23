// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');

goog.scope(function() {

const KeywordPainter = highlights.KeywordPainter;

/** @const @type {!Set<string>} */
const staticJavascriptKeywords =
    highlights.HighlightEngines.keywordsFor('javascript');

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

// Export |JavascriptPainter| for testing.
/** @constructor */
highlights.JavascriptPainter = JavascriptPainter;
});
