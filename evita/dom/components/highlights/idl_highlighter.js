// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights.base');

goog.scope(function() {

const IdlTokenStateMachine = highlights.IdlTokenStateMachine;
const Highlighter = highlights.base.Highlighter;
const KeywordPainter = highlights.base.KeywordPainter;
const Token = highlights.base.Token;

/** @const @type {!Set<string>} */
const staticIdlKeywords = highlights.staticIdlKeywords;

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
