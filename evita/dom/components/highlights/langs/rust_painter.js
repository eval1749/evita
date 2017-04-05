// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');

goog.scope(function() {

const KeywordPainter = highlights.KeywordPainter;
const Token = highlights.Token;

/** @const @type {!Set<string>} */
const staticRustKeywords = highlights.HighlightEngine.keywordsFor('rust');

class RustPainter extends KeywordPainter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document, staticRustKeywords); }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!RustPainter}
   */
  static create(document) { return new RustPainter(document); }
}

// Export |RustPainter| for testing.
/** @constructor */
highlights.RustPainter = RustPainter;
});
