// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');

goog.scope(function() {

const KeywordPainter = highlights.KeywordPainter;

/** @const @type {!Set<string>} */
const staticGnKeywords = highlights.HighlightEngines.keywordsFor('gn');

class GnPainter extends KeywordPainter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document, staticGnKeywords); }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!GnPainter}
   */
  static create(document) { return new GnPainter(document); }
}

// Export |GnPainter| for testing.
/** @constructor */
highlights.GnPainter = GnPainter;
});
