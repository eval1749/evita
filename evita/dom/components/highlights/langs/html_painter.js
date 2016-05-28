// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');
goog.require('highlights.TagPainter');

goog.scope(function() {

const Painter = highlights.Painter;
const TagPainter = highlights.TagPainter;
const Token = highlights.Token;
const Tokenizer = highlights.Tokenizer;

/** @const @type {!Set<string>} */
const staticHtmlKeywords = highlights.HighlightEngine.keywordsFor('html');

class HtmlPainter extends Painter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document);
    /** @const @type {!Tokenizer} */
    this.cssPainter_ = highlights.createCssTokenizer(document);
    /** @const @type {!Tokenizer} */
    this.scriptPainter_ = highlights.createJavaScriptTokenizer(document);
    /** @const @type {!TagPainter} */
    this.tagPainter_ = new TagPainter(document, staticHtmlKeywords);
  }

  /**
   * @override
   * @public
   * @param {number} headCount
   * @param {number} tailCount
   * @param {number} delta
   */
  didChangeTextDocument(headCount, tailCount, delta) {
    super.didChangeTextDocument(headCount, tailCount, delta);
    this.cssPainter_.didChangeTextDocument(headCount, tailCount, delta);
    this.scriptPainter_.didChangeTextDocument(headCount, tailCount, delta);
  }

  /**
   * @override
   * @public
   */
  didLoadTextDocument() {
    super.didLoadTextDocument();
    this.cssPainter_.didLoadTextDocument();
    this.scriptPainter_.didLoadTextDocument();
  }

  /** @override */
  didChangeVerbose() {
    this.cssPainter_.verbose = this.verbose;
    this.scriptPainter_.verbose = this.verbose;
  }

  /**
   * @private
   * @param {!Token} token
   */
  findEndOfContent(token) {
    if (this.document.charCodeAt(token.end - 1) !== Unicode.GREATER_THAN_SIGN)
      return token.end;
    /** @type {number} */
    let runner = token.end - 1;
    for (;;) {
      while (this.document.charCodeAt(runner) !== Unicode.SOLIDUS)
        --runner;
      --runner;
      if (this.document.charCodeAt(runner) === Unicode.LESS_THAN_SIGN)
        return runner;
    }
  }

  /**
   * @override
   * @param {!Token} token
   */
  paint(token) {
    switch (token.syntax) {
      case 'content':
        return this.paintToken2(token, 'normal');
      case 'endTag':
        return this.tagPainter_.paintEndTag(token);
      case 'script':
        return this.paintScript(token);
      case 'startTag':
        return this.tagPainter_.paintStartTag(token);
      case 'style':
        return this.paintStyle(token);
    }
    super.paint(token);
  }

  /**
   * @private
   * @param {!Tokenizer} painter
   * @param {!Token} token
   */
  paintNonHtmlContent(painter, token) {
    /** @const @type {number} */
    const contentEnd = this.findEndOfContent(token);
    painter.process(token.start, contentEnd, token.start, contentEnd);
    if (contentEnd === token.end) {
      // We've not seen end tag yet.
      return;
    }
    this.tagPainter_.paintEndTag2(contentEnd, token.end);
  }

  /**
   * @private
   * @param {!Token} token
   */
  paintScript(token) { this.paintNonHtmlContent(this.scriptPainter_, token); }

  /**
   * @private
   * @param {!Token} token
   */
  paintStyle(token) { this.paintNonHtmlContent(this.cssPainter_, token); }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!HtmlPainter}
   */
  static create(document) { return new HtmlPainter(document); }
}

/** @constructor */
highlights.HtmlPainter = HtmlPainter;
});
