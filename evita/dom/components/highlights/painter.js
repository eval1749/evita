// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('highlights.Paint');
goog.provide('highlights.KeywordPaint');

goog.require('highlights');

goog.scope(function() {

const Token = highlights.Token;

/*
 * |Painter| provides basic functionality for setting syntax for |TextDocument|
 * with |Token|.
 */
class Painter {
  /**
   * @protected
   * @param {!TextDocument} document
   */
  constructor(document) {
    /** @const @type {!TextDocument} */
    this.document_ = document;
  }

  /**
   * @public
   * @param {number} headCount
   * @param {number} tailCount
   * @param {number} delta
   * Exposed for embed language tokenizer.
   */
  didChangeTextDocument(headCount, tailCount, delta) {}

  /**
   * @public
   * Exposed for embed language tokenizer.
   */
  didLoadTextDocument() {}

  /** @return {!TextDocument} */
  get document() { return this.document_; }

  /**
   * @protected
   * @param {!Token} token
   */
  paintToken(token) { this.paintToken2(token, token.syntax); }

  /**
   * @protected
   * @param {!Token} token
   * @param {string} syntax
   */
  paintToken2(token, syntax) { this.setSyntax(token.start, token.end, syntax); }

  /**
   * @public
   * @param {!Token} token
   */
  paint(token) { this.paintToken(token); }

  /**
   * @public
   * @param {number} start
   * @param {number} end
   * @param {string} syntax
   */
  setSyntax(start, end, syntax) {
    console.assert(start < end, start, end, syntax);
    this.document_.setSyntax(start, end, syntax);
  }

  /**
   * @protected
   * @param {!Token} token
   * @return {string}
   */
  textOf(token) { return this.document_.slice(token.start, token.end); }
}

/*
 * Painter with keyword support
 */
class KeywordPainter extends Painter {
  /**
   * @protected
   * @param {!TextDocument} document
   * @param {!Set<string>} keywords
   * @param {string=} suffixChar
   */
  constructor(document, keywords, suffixChar = '') {
    super(document);
    console.assert(keywords.size >= 1);
    /** @const @type {!Set<string>} */
    this.keywords_ = keywords;
    /** @const @type {string} */
    this.delimiter = suffixChar;
  }

  /**
   * @override
   * @param {!Token} token
   */
  paint(token) {
    if (token.syntax !== 'identifier' || token.length == 1)
      return this.paintToken(token);
    /** @const @type {string} */
    const fullName = this.textOf(token);
    if (this.keywords_.has(fullName))
      return this.paintToken2(token, 'keyword');
    if (this.delimiter === '')
      return this.paintToken(token);

    /** @const @type {number} */
    const headIndex = fullName.lastIndexOf(this.delimiter);
    if (headIndex <= 0)
      return this.paintToken(token);

    /** @const @type {number} */
    const headEnd = token.start + headIndex;
    /** @const @type {string} */
    const head = fullName.substr(0, headIndex);
    if (this.keywords_.has(head))
      this.setSyntax(token.start, headEnd, 'keyword');
    else
      this.setSyntax(token.start, headEnd, token.syntax);

    /** @const @type {number} */
    const tailIndex = fullName.lastIndexOf(this.delimiter);
    /** @const @type {string} */
    const tail = fullName.substr(tailIndex);
    if (!this.keywords_.has(tail))
      return this.setSyntax(headEnd, token.end, token.syntax);

    // Paint tail as keyword
    /** @const @type {number} */
    const tailStart = token.start + tailIndex;
    if (headEnd != tailStart)
      this.setSyntax(headEnd, tailStart, token.syntax);
    this.setSyntax(tailStart, tailStart + 1, 'operator');
    this.setSyntax(tailStart + 1, token.end, 'keyword');
  }
}

const namespace = highlights;
/** @constructor */
namespace.KeywordPainter = KeywordPainter;
/** @constructor */
namespace.Painter = Painter;
});
