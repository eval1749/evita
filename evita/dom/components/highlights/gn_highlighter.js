// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights.base');

goog.scope(function() {

const GnTokenStateMachine = highlights.GnTokenStateMachine;
const Highlighter = highlights.base.Highlighter;
const Painter = highlights.base.Painter;
const Token = highlights.base.Token;
const Tokenizer = highlights.base.Tokenizer;

/** @const @type {!Set<string>} */
const staticKeywords = highlights.staticGnKeywords;

class GnPainter extends Painter {
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
    const name = this.textOf(token);
    if (staticKeywords.has(name)) {
      return this.paintToken2(token, 'keyword');
    }
    this.paintToken(token);
  }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!Painter}
   */
  static create(document) { return new GnPainter(document); }
}

class GnHighlighter extends Highlighter {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document, GnPainter.create, new GnTokenStateMachine());
  }
}

highlights.GnHighlighter = GnHighlighter;
// Export |GnPainter| for testing.
highlights.GnPainter = GnPainter;
});

// Override |GnLexer| by |GnHighlighter|.
// TODO(eval1749): Once we get rid of |GnLexer|, we should get rid of this
// override.
global['GnLexer'] = highlights.GnHighlighter;
