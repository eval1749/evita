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
const TokenMap = highlights.base.TokenMap;

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
   * @param {!TokenMap} tokenMap
   */
  constructor(document, tokenMap) { super(document, tokenMap); }

  /**
   * @param {!Token} token
   * @return {string}
   */
  colorOfTag(token) {
    if (token.length > 1)
      return 'html_element_name';
    const charCode = this.document.charCodeAt(token.start);
    if (charCode === Unicode.GREATER_THAN_SIGN ||
        charCode === Unicode.LESS_THAN_SIGN || charCode === Unicode.SOLIDUS) {
      return 'keyword';
    }
    return 'html_element_name';
  }

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
    this.document.setSyntax(token.start, token.end, token.syntax);
  }

  /**
   * @private
   * @param {!Token} token
   */
  paintEndTag(token) {
    this.document.setSyntax(token.start, token.end, this.colorOfTag(token));
  }

  /**
   * @private
   * @param {!Token} token
   */
  paintScript(token) {
    this.document.setSyntax(token.start, token.end, 'html_attribute_name');
  }

  /**
   * @private
   * @param {!Token} token
   */
  paintStyle(token) {
    this.document.setSyntax(token.start, token.end, 'html_attribute_name');
  }

  /**
   * @private
   * @param {!Token} token
   */
  paintStartTag(token) {
    this.document.setSyntax(token.start, token.end, this.colorOfTag(token));
    // TODO(eval1749): NYI: color HTML attributes
  }

  /**
   * @public
   * @param {!TextDocument} document
   * @param {!TokenMap} tokenMap
   * @return {!Painter}
   */
  static create(document, tokenMap) {
    return new HtmlPainter(document, tokenMap);
  }
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
