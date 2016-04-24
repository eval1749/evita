// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');
goog.require('highlights.TagPainter');

goog.scope(function() {

const Painter = highlights.Painter;
const TagPainter = highlights.TagPainter;
const Token = highlights.Token;

/** @const @type {!Set<string>} */
const staticXmlKeywords = highlights.HighlightEngine.keywordsFor('xml');

class XmlPainter extends Painter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document);
    /** @const @type {!TagPainter} */
    this.tagPainter_ = new TagPainter(document, staticXmlKeywords);
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
        return this.tagPainter_.paintEndTag(token);
      case 'pi':
        return this.paintPi(token);
      case 'startTag':
        return this.tagPainter_.paintStartTag(token);
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
    this.tagPainter_.paintTag(contentStart, contentEnd);
  }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!Painter}
   */
  static create(document) { return new XmlPainter(document); }
}

/** @constructor */
highlights.XmlPainter = XmlPainter;
});
