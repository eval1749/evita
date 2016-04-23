// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('highlights.xml');

goog.require('highlights.base');

goog.scope(function() {

const XmltagTokenStateMachine = highlights.XmltagTokenStateMachine;
const XmlTokenStateMachine = highlights.XmlTokenStateMachine;
const Highlighter = highlights.base.Highlighter;
const Painter = highlights.base.Painter;
const Token = highlights.base.Token;
const Tokenizer = highlights.base.Tokenizer;

/** @const @type {!Set<string>} */
const staticXmlKeywords = new Set();

class TagPainter {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    /** @const @type {!TextDocument} */
    this.document_ = document;
    /** @type {string} */
    this.lastLabel_ = '';
    /** @const @type {!XmltagTokenStateMachine} */
    this.stateMachine_ = new XmltagTokenStateMachine();
    /** @type {string} */
    this.syntaxForWord_ = 'html_element_name';
  }

  /**
   * @private
   * @param {number} start
   * @param {number} end
   */
  paintString(start, end) {
    this.setSyntax(start, start + 1, 'keyword');
    if (end - start <= 1)
      return;
    /** @const @type {number} */
    const quote = this.document_.charCodeAt(start);
    if (quote !== this.document_.charCodeAt(end - 1)) {
      // string literal doesn't end.
      this.setSyntax(start + 1, end, 'html_attribute_value');
      return;
    }
    this.setSyntax(end - 1, end, 'keyword');
    if (end - start === 2) {
      // Empty string; no characters between quotation mark
      return;
    }
    this.setSyntax(start + 1, end - 1, 'html_attribute_value');
  }

  /**
   * @public
   * @param {number} start
   * @param {number} end
   */
  paintTag(start, end) {
    this.reset();
    /** @type {number} */
    let tokenStart = start;
    /** @type {string} */
    let tokenLabel = '';
    for (let offset = start; offset < end; ++offset) {
      /** @const @type {number} */
      const charCode = this.document_.charCodeAt(offset);
      /** @const @type {number} */
      const oldState = this.stateMachine_.state;
      /** const @type {number} */
      const newState = this.updateState(charCode);
      if (oldState === newState)
        continue;
      /** @const @type {string} */
      const label = this.stateMachine_.syntaxOf(newState);
      if (tokenLabel === label)
        continue;
      if (tokenLabel === '')
        tokenLabel = label;
      this.paintToken(tokenStart, offset, tokenLabel);
      tokenStart = offset;
      tokenLabel = label;
    }
    this.paintToken(tokenStart, end, tokenLabel);
  }

  /**
   * @private
   * @param {number} start
   * @param {number} end
   * @param {string} label
   */
  paintToken(start, end, label) {
    if (start === end)
      return;
    switch (label) {
      case 'equal':
        this.setSyntax(start, end, 'keyword');
        break;
      case 'string':
        this.paintString(start, end);
        break;
      case 'word':
        this.paintWord(start, end);
        break;
      default:
        this.setSyntax(start, end, 'normal');
        break;
    }
    this.lastLabel_ = label;
  }

  /**
   * @private
   * @param {number} start
   * @param {number} end
   */
  paintWord(start, end) {
    /** @const @type {string} */
    const syntax = this.lastLabel_ === 'equal' ? 'html_attribute_value' :
                                                 this.syntaxForWord_;
    this.setSyntax(start, end, syntax);
    this.syntaxForWord_ = 'html_attribute_name';
  }

  /**
   * @private
   * @param {number} start
   * @param {number} end
   * @param {string} syntax
   */
  setSyntax(start, end, syntax) {
    console.assert(start < end, start, end, syntax);
    this.document_.setSyntax(start, end, syntax);
  }

  /** @private */
  reset() {
    this.lastLabel_ = '';
    this.stateMachine_.resetTo(0);
    this.syntaxForWord_ = 'html_element_name';
  }

  /**
   * @private
   * @param {number} charCode
   * @return {number}
   */
  updateState(charCode) {
    /** @const @type {number} */
    const newState = this.stateMachine_.updateState(charCode);
    if (newState !== 0)
      return newState;
    return this.stateMachine_.updateState(charCode);
  }
}

class XmlPainter extends Painter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document);
    /** @const @type {!TagPainter} */
    this.tagPainter_ = new TagPainter(document);
  }

  /**
   * @override
   * @param {!Token} token
   */
  paint(token) {
    switch (token.syntax) {
      case 'cdata':
        return this.paintCdata(token);
      case 'content':
        return this.paintToken2(token, 'normal');
      case 'endTag':
        return this.paintEndTag(token);
      case 'pi':
        return this.paintPi(token);
      case 'startTag':
        return this.paintStartTag(token);
    }
    this.paintToken(token);
  }

  /**
   * @private
   * @param {!Token} token
   */
  paintCdata(token) {
    /** @const @type {number} */
    const start = token.start;
    /** @const @type {number} */
    const end = token.end;
    /** @const @type {number} */
    const contentStart = Math.min(start + 9, end);
    /** @const @type {number} */
    const contentEnd = Math.max(contentStart, end - 3);
    // Paint "<![CDATA["
    this.setSyntax(start, contentStart, 'keyword');
    if (contentStart === contentEnd)
      return;
    this.setSyntax(contentStart, contentEnd, 'string_literal');
    if (contentEnd === end)
      return;
    // Paint "]]>"
    this.setSyntax(contentEnd, end, 'keyword');
  }

  /**
   * @private
   * @param {!Token} token
   */
  paintEndTag(token) {
    /** @const @type {number} */
    const start = token.start;
    /** @const @type {number} */
    const end = token.end;
    this.setSyntax(start, start + 2, 'keyword');
    if (start + 2 == end)
      return;
    if (this.document.charCodeAt(end - 1) !== Unicode.GREATER_THAN_SIGN)
      return this.setSyntax(start + 2, end, 'html_element_name');
    this.setSyntax(start + 2, end - 1, 'html_element_name');
    this.setSyntax(end - 1, end, 'keyword');
  }

  /**
   * @private
   * @param {!Token} token
   */
  paintPi(token) {
    /** @const @type {number} */
    const start = token.start;
    /** @const @type {number} */
    const end = token.end;
    /** @const @type {number} */
    const contentStart = Math.min(start + 2, end);
    /** @const @type {number} */
    const contentEnd = Math.max(contentStart, end - 2);
    this.setSyntax(start, contentStart, 'keyword');
    if (contentEnd !== end)
      this.setSyntax(contentEnd, end, 'keyword');
    this.paintTag(contentStart, contentEnd);
  }

  /**
   * @private
   * @param {!Token} token
   */
  paintStartTag(token) {
    if (token.length < 3)
      return;
    /** @const @type {number} */
    const start = token.start;
    /** @const @type {number} */
    const end = token.end;
    this.setSyntax(start, start + 1, 'keyword');
    if (this.document.charCodeAt(end - 1) !== Unicode.GREATER_THAN_SIGN)
      return this.paintTag(start + 1, end);
    if (this.document.charCodeAt(end - 2) === Unicode.SOLIDUS) {
      this.setSyntax(end - 2, end, 'keyword');
      return this.paintTag(start + 1, end - 2);
    }
    this.setSyntax(end - 1, end, 'keyword');
    return this.paintTag(start + 1, end - 1);
  }

  /**
   * @private
   * @param {number} start
   * @param {number} end
   */
  paintTag(start, end) { this.tagPainter_.paintTag(start, end); }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!Painter}
   */
  static create(document) { return new XmlPainter(document); }
}

class XmlHighlighter extends Highlighter {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document, XmlPainter.create, new XmlTokenStateMachine());
  }

  /**
   * @public
   * @return {!Set<string>}
   * For debugging
   */
  static get keywords() { return staticXmlKeywords; }

  /**
   * @public
   * @param {string} word
   * Adds a keyword at runtime.
   */
  static addKeyword(word) { staticXmlKeywords.add(word); }
}

['xi:include', 'xml:base', 'xml:lang', 'xmlns:', 'xml:space']
    .forEach(word => XmlHighlighter.addKeyword(word));

/** @constructor */
highlights.xml.XmlHighlighter = XmlHighlighter;

/** @constructor */
highlights.xml.XmlPainter = XmlPainter;
});

// Override |XmlLexer| by |XmlHighlighter|.
// TODO(eval1749): Once we get rid of |XmlLexer|, we should get rid of this
// override.
global['XmlLexer'] = highlights.xml.XmlHighlighter;
