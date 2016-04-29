// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');

goog.scope(function() {

const KeywordPainter = highlights.KeywordPainter;

/** @const @type {!Set<string>} */
const staticJavaKeywords = highlights.HighlightEngine.keywordsFor('java');

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

// Export |JavaPainter| for testing.
/** @constructor */
highlights.JavaPainter = JavaPainter;
});
