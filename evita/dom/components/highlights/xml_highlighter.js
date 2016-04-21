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

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isWhitespace(charCode) {
  return charCode <= Unicode.SPACE;
}

class TagPainter {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    /** @const @type {!TextDocument} */
    this.document_ = document;
    /** @type {string} */
    this.syntaxForName_ = 'html_element_name';
  }

  /**
   * @private
   * @param {number} start
   * @param {number} end
   * @param {string} label
   */
  paint(start, end, label) {
    if (start === end)
      return;
    switch (label) {
      case 'name':
        return this.paintName(start, end);
      case 'string':
        return this.paintString(start, end);
      case 'value':
        return this.paintValue(start, end);
    }
    this.document_.setSyntax(start, end, 'normal');
  }

  /**
   * @private
   * @param {number} start
   * @param {number} end
   */
  paintName(start, end) {
    this.document_.setSyntax(start, end, this.syntaxForName_);
    this.syntaxForName_ = 'html_attribute_name';
  }

  /**
   * @private
   * @param {number} start
   * @param {number} end
   */
  paintString(start, end) {
    this.document_.setSyntax(start, start + 1, 'normal');
    if (end - start <= 1)
      return;
    this.document_.setSyntax(end - 1, end, 'normal');
    this.document_.setSyntax(start + 1, end - 1, 'html_attribute_value');
  }

  /**
   * @private
   * @param {number} start
   * @param {number} end
   */
  paintValue(start, end) {
    this.document_.setSyntax(start, start + 1, 'normal');
    if (start + 1 === end)
      return;
    this.document_.setSyntax(start + 1, end, 'html_attribute_value');
  }

  /**
   * @private
   * @param {number} start
   * @param {number} end
   */
  run(start, end) {
    /** @type {number} */
    let tokenStart = start;
    /** @type {string} */
    let tokenLabel = '';
    /** @const @type {!XmltagTokenStateMachine} */
    const stateMachine = new XmltagTokenStateMachine();
    for (let offset = start; offset < end; ++offset) {
      /** @const @type {number} */
      const charCode = this.document_.charCodeAt(offset)
                       /** @const @type {number} */
                       const oldState = stateMachine.state;
      /** @const @type {number} */
      const newState = stateMachine.updateState(charCode);
      if (oldState === newState)
        continue;
      /** @const @type {string} */
      const label = stateMachine.syntaxOf(newState);
      if (tokenLabel === label)
        continue;
      this.paint(tokenStart, offset, tokenLabel);
      tokenStart = offset;
      tokenLabel = label;
    }
    this.paint(tokenStart, end, tokenLabel);
  }
}

class XmlPainter extends Painter {
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
    this.document.setSyntax(start, contentStart, 'keyword');
    if (contentStart === contentEnd)
      return;
    this.document.setSyntax(contentStart, contentEnd, 'string_literal');
    if (contentEnd === end)
      return;
    // Paint "]]>"
    this.document.setSyntax(contentEnd, end, 'keyword');
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
  paintPi(token) {
    /** @const @type {number} */
    const start = token.start;
    /** @const @type {number} */
    const end = token.end;
    /** @const @type {number} */
    const contentStart = Math.min(start + 2, end);
    /** @const @type {number} */
    const contentEnd = Math.max(contentStart, end - 2);
    this.document.setSyntax(start, contentStart, 'keyword');
    if (contentEnd !== end)
      this.document.setSyntax(contentEnd, end, 'keyword');
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
    this.document.setSyntax(start, start + 1, 'keyword');
    if (this.document.charCodeAt(end - 1) !== Unicode.GREATER_THAN_SIGN)
      return this.paintTag(start + 1, end);
    if (this.document.charCodeAt(end - 2) === Unicode.SOLIDUS) {
      this.document.setSyntax(end - 2, end, 'keyword');
      return this.paintTag(start + 1, end - 2);
    }
    this.document.setSyntax(end - 1, end, 'keyword');
    return this.paintTag(start + 1, end - 1);
  }

  /**
   * @private
   * @param {number} start
   * @param {number} end
   */
  paintTag(start, end) { (new TagPainter(this.document)).run(start, end); }

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

/** @constructor */
highlights.xml.XmlHighlighter = XmlHighlighter;

/** @constructor */
highlights.xml.XmlPainter = XmlPainter;
});

// Override |XmlLexer| by |XmlHighlighter|.
// TODO(eval1749): Once we get rid of |XmlLexer|, we should get rid of this
// override.
global['XmlLexer'] = highlights.xml.XmlHighlighter;
