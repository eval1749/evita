// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights.base');

goog.scope(function() {

const IdlTokenStateMachine = highlights.IdlTokenStateMachine;
const Highlighter = highlights.base.Highlighter;
const Painter = highlights.base.Painter;
const Token = highlights.base.Token;

/** @const @type {!Set<string>} */
const staticIdlKeywords = highlights.staticIdlKeywords;

class IdlPainter extends Painter {
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
    if (staticIdlKeywords.has(name)) {
      return this.paintToken2(token, 'keyword');
    }
    this.paintToken(token);
  }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!Painter}
   */
  static create(document) { return new IdlPainter(document); }
}

class IdlHighlighter extends Highlighter {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document, IdlPainter.create, new IdlTokenStateMachine());
  }

  /**
   * @public
   * @return {!Set<string>}
   * For debugging
   */
  static get keywords() { return staticIdlKeywords; }

  /**
   * @public
   * @param {string} word
   * Adds a keyword at runtime.
   */
  static addKeyword(word) { staticIdlKeywords.add(word); }
}

/** @constructor */
highlights.IdlHighlighter = IdlHighlighter;
// Export |IdlPainter| for testing.
/** @constructor */
highlights.IdlPainter = IdlPainter;
});

// Override |IdlLexer| by |IdlHighlighter|.
// TODO(eval1749): Once we get rid of |IdlLexer|, we should get rid of this
// override.
global['IdlLexer'] = highlights.IdlHighlighter;
