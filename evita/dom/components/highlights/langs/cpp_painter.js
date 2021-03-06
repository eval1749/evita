// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');

goog.scope(function() {

const Painter = highlights.Painter;
const Token = highlights.Token;

/** @const @type {!Set<string>} */
const staticCppKeywords = highlights.HighlightEngine.keywordsFor('c++');

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isWhitespace(charCode) {
  return charCode <= Unicode.SPACE;
}

class CppPainter extends Painter {
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

    if (this.document.charCodeAt(token.start) === Unicode.NUMBER_SIGN) {
      /** @type {number} */
      let start = token.start + 1;
      while (start < token.end) {
        if (!isWhitespace(this.document.charCodeAt(start)))
          break;
        ++start;
      }
      if (start === token.end)
        return this.paintToken(token);
      /** @const @type {string} */
      const name = '#' + this.document.slice(start, token.end);
      if (staticCppKeywords.has(name))
        return this.paintToken2(token, 'keyword');
      this.setSyntax(token.start, start, '#');
      this.setSyntax(token.start + 1, token.end, 'identifier');
      return;
    }

    /** @const @type {string} */
    const name = this.textOf(token);

    if (this.document.charCodeAt(token.end - 1) === Unicode.COLON &&
        this.document.charCodeAt(token.end - 2) !== Unicode.COLON) {
      /** @const @type {string} */
      const syntax = staticCppKeywords.has(name) ? 'keyword' : 'label';
      this.setSyntax(token.start, token.end - 1, syntax);
      this.setSyntax(token.end - 1, token.end, 'operator');
      return;
    }

    /** @const @type {string} */
    const syntax = staticCppKeywords.has(name) ? 'keyword' : 'identifier';
    this.paintToken2(token, syntax);
  }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!Painter}
   */
  static create(document) { return new CppPainter(document); }
}

// Export |CppPainter| for testing.
/** @constructor */
highlights.CppPainter = CppPainter;
});
