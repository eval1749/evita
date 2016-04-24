// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');

goog.scope(function() {

const Painter = highlights.Painter;
const Token = highlights.Token;

/** @const @type {!Set<string>} */
const staticCssKeywords = highlights.HighlightEngine.keywordsFor('css');

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isWhitespace(charCode) {
  return charCode <= Unicode.SPACE;
}

class CssPainter extends Painter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document); }

  /**
   * @override
   * @param {!Token} token
   * This function does following custom painting for C++ language:
   *  - label, name ends with ":".
   *  - C++ keywords
   *  - qualified name; namespace? "::" name. (one namespace prefix only)
   *
   */
  paint(token) {
    if (token.syntax !== 'identifier' || token.length == 1)
      return this.paintToken(token);

    /** @const @type {string} */
    const name = this.textOf(token);
    if (staticCssKeywords.has(name))
      return this.paintToken2(token, 'keyword');
    if (this.document.charCodeAt(token.end - 1) === Unicode.COLON)
      return this.paintToken2(token, 'label');
    this.paintToken2(token, 'identifier');
  }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!Painter}
   */
  static create(document) { return new CssPainter(document); }
}

// Export |CssPainter| for testing.
/** @constructor */
highlights.CssPainter = CssPainter;
});
