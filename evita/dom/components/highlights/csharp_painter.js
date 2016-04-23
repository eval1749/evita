// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');

goog.scope(function() {

const KeywordPainter = highlights.KeywordPainter;

/** @const @type {!Set<string>} */
const staticKeywords = highlights.HighlightEngines.keywordsFor('c#');

class CSharpPainter extends KeywordPainter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document, staticKeywords, '.'); }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!CSharpPainter}
   */
  static create(document) { return new CSharpPainter(document); }
}

// Export |CSharpPainter| for testing.
/** @constructor */
highlights.CSharpPainter = CSharpPainter;
});
