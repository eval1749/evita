// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');

goog.scope(function() {

const Painter = highlights.Painter;
const Token = highlights.Token;

/** @const @type {!Set<string>} */
const staticPythonKeywords = highlights.HighlightEngines.keywordsFor('python');

class PythonPainter extends Painter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document); }

  /**
   * @override
   * @param {!Token} token
   */
  paint(token) {
    if (token.syntax !== 'identifier' || token.length == 1)
      return this.paintToken(token);
    /** @const @type {string} */
    const fullName = this.textOf(token);
    if (staticPythonKeywords.has(fullName))
      return this.paintToken2(token, 'keyword');
    const dotIndex = fullName.indexOf('.');
    if (dotIndex < 0 || !staticPythonKeywords.has(fullName.substr(0, dotIndex)))
      return this.paintToken(token);
    /** @const @type {number} */
    const nameEnd = token.start + dotIndex;
    this.setSyntax(token.start, nameEnd, 'keyword');
    this.setSyntax(nameEnd, token.end, 'identifier');
  }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!Painter}
   */
  static create(document) { return new PythonPainter(document); }
}

/** @constructor */
highlights.PythonPainter = PythonPainter;
});
