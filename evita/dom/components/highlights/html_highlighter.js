// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights.base');

goog.scope(function() {

const HtmlTokenStateMachine = highlights.HtmlTokenStateMachine;
const Highlighter = highlights.base.Highlighter;
const Painter = highlights.base.Painter;
const Token = highlights.base.Token;
const Tokenizer = highlights.base.Tokenizer;

/** @const @type {!Set<string>} */
const staticHtmlKeywords = new Set();

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isWhitespace(charCode) {
  return charCode <= Unicode.SPACE;
}

class HtmlPainter extends Painter {
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
    switch (token.syntax) {
      case 'endTag':
        return this.paintEndTag(token);
      case 'script':
        return this.paintStyle(token);
      case 'startTag':
        return this.paintStartTag(token);
      case 'style':
        return this.paintStyle(token);
    }
    this.paintToken(token);
  }

  /**
   * @private
   * @param {!Token} token
   */
  paintEndTag(token) {
    const start = token.start;
    const end = token.end;
    this.document.setSyntax(start, start + 2, 'keyword');
    if (start + 2 == end)
      return;
    if (this.document.charCodeAt(end - 1) !== Unicode.GREATER_THAN_SIGN)
      return this.document.setSyntax(start + 2, end, 'html_element_name');
    this.document.setSyntax(start + 2, end - 1, 'html_element_name');
    this.document.setSyntax(end - 1, end, 'keyword');
  }

  /**
   * @private
   * @param {!Token} token
   */
  paintScript(token) { this.paintToken2(token, 'html_attribute_name'); }

  /**
   * @private
   * @param {!Token} token
   */
  paintStyle(token) { this.paintToken2(token, 'html_attribute_name'); }

  /**
   * @private
   * @param {!Token} token
   */
  paintStartTag(token) {
    if (token.length < 3)
      return;
    const start = token.start;
    const end = token.end;
    this.document.setSyntax(start, start + 1, 'keyword');
    if (this.document.charCodeAt(end - 1) !== Unicode.GREATER_THAN_SIGN)
      return this.document.setSyntax(start + 1, end, 'html_element_name');
    this.document.setSyntax(start + 1, end - 1, 'html_element_name');
    // TODO(eval1749): NYI: color HTML attributes
    this.document.setSyntax(end - 1, end, 'keyword');
  }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!Painter}
   */
  static create(document) { return new HtmlPainter(document); }
}

class HtmlHighlighter extends Highlighter {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document, HtmlPainter.create, new HtmlTokenStateMachine());
  }

  /**
   * @public
   * @return {!Set<string>}
   * For debugging
   */
  static get keywords() { return staticHtmlKeywords; }

  /**
   * @public
   * @param {string} word
   * Adds a keyword at runtime.
   */
  static addKeyword(word) { staticHtmlKeywords.add(word); }
}

Object.freeze(HtmlHighlighter);
Object.seal(HtmlHighlighter);

highlights.HtmlHighlighter = HtmlHighlighter;
});

// Override |HtmlLexer| by |HtmlHighlighter|.
// TODO(eval1749): Once we get rid of |HtmlLexer|, we should get rid of this
// override.
global['HtmlLexer'] = highlights.HtmlHighlighter;
