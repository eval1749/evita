// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights.base');

goog.scope(function() {

const HtmlTokenStateMachine = highlights.HtmlTokenStateMachine;
const Highlighter = highlights.base.Highlighter;
const Painter = highlights.base.Painter;
const StateRange = highlights.base.StateRange;
const Tokenizer = highlights.base.Tokenizer;
const StateRangeMap = highlights.base.StateRangeMap;

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
   * @param {!StateRangeMap} rangeMap
   */
  constructor(document, rangeMap) { super(document, rangeMap); }

  /**
   * @param {!StateRange} range
   * @return {string}
   */
  colorOfTag(range) {
    if (range.length > 1)
      return 'html_element_name';
    const charCode = this.document.charCodeAt(range.start);
    if (charCode === Unicode.GREATER_THAN_SIGN ||
        charCode === Unicode.LESS_THAN_SIGN || charCode === Unicode.SOLIDUS) {
      return 'keyword';
    }
    return 'html_element_name';
  }

  /**
   * @override
   * @param {!StateRange} range
   */
  paint(range) {
    switch (range.syntax) {
      case 'endTag':
        return this.paintEndTag(range);
      case 'script':
        return this.paintStyle(range);
      case 'startTag':
        return this.paintStartTag(range);
      case 'style':
        return this.paintStyle(range);
    }
    this.document.setSyntax(range.start, range.end, range.syntax);
  }

  /**
   * @private
   * @param {!StateRange} range
   */
  paintEndTag(range) {
    this.document.setSyntax(range.start, range.end, this.colorOfTag(range));
  }

  /**
   * @private
   * @param {!StateRange} range
   */
  paintScript(range) {
    this.document.setSyntax(range.start, range.end, 'html_attribute_name');
  }

  /**
   * @private
   * @param {!StateRange} range
   */
  paintStyle(range) {
    this.document.setSyntax(range.start, range.end, 'html_attribute_name');
  }

  /**
   * @private
   * @param {!StateRange} range
   */
  paintStartTag(range) {
    this.document.setSyntax(range.start, range.end, this.colorOfTag(range));
    // TODO(eval1749): NYI: color HTML attributes
  }

  /**
   * @public
   * @param {!TextDocument} document
   * @param {!StateRangeMap} rangeMap
   * @return {!Painter}
   */
  static create(document, rangeMap) {
    return new HtmlPainter(document, rangeMap);
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
