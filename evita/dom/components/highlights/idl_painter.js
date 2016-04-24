// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');

goog.scope(function() {

const KeywordPainter = highlights.KeywordPainter;
const Token = highlights.Token;

/** @const @type {!Set<string>} */
const staticIdlKeywords = highlights.HighlightEngine.keywordsFor('idl');

class IdlPainter extends KeywordPainter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document, staticIdlKeywords); }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!IdlPainter}
   */
  static create(document) { return new IdlPainter(document); }
}

// Export |IdlPainter| for testing.
/** @constructor */
highlights.IdlPainter = IdlPainter;
});
