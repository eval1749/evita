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
const staticHtmlKeywords = highlights.HighlightEngines.keywordsFor('html');

class HtmlPainter extends Painter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document);
    /** @const @type {!TagPainter} */
    this.tagPainter_ = new TagPainter(document, staticHtmlKeywords);
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
        return this.paintStyle(token);
      case 'startTag':
        return this.tagPainter_.paintStartTag(token);
      case 'style':
        return this.paintStyle(token);
    }
    super.paint(token);
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
   * @public
   * @param {!TextDocument} document
   * @return {!HtmlPainter}
   */
  static create(document) { return new HtmlPainter(document); }
}

/** @constructor */
highlights.HtmlPainter = HtmlPainter;
});
