// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights.base');

goog.scope(function() {

const PythonTokenStateMachine = highlights.PythonTokenStateMachine;
const Highlighter = highlights.base.Highlighter;
const Painter = highlights.base.Painter;
const Token = highlights.base.Token;
const Tokenizer = highlights.base.Tokenizer;

/** @const @type {!Set<string>} */
const staticPythonKeywords = highlights.staticPythonKeywords;

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

class PythonHighlighter extends Highlighter {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document, PythonPainter.create, new PythonTokenStateMachine());
  }

  /**
   * @public
   * @return {!Set<string>}
   * For debugging
   */
  static get keywords() { return staticPythonKeywords; }

  /**
   * @public
   * @param {string} word
   * Adds a keyword at runtime.
   */
  static addKeyword(word) { staticPythonKeywords.add(word); }
}

/** @constructor */
highlights.PythonHighlighter = PythonHighlighter;

/** @constructor */
highlights.PythonPainter = PythonPainter;
});

// Override |PythonLexer| by |PythonHighlighter|.
// TODO(eval1749): Once we get rid of |PythonLexer|, we should get rid of this
// override.
global['PythonLexer'] = highlights.PythonHighlighter;
