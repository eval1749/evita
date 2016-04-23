// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('highlights.TagPainter');

goog.scope(function() {

const Token = highlights.Token;
const XmltagTokenStateMachine = highlights.XmltagTokenStateMachine;

class TagPainter {
  /**
   * @param {!TextDocument} document
   * @param {!Set<string>} keywords
   */
  constructor(document, keywords) {
    /** @const @type {!TextDocument} */
    this.document_ = document;
    /** @type {boolean} */
    this.isTagName_ = true;
    /** @const @type {!Set<string>} */
    this.keywords_ = keywords;
    /** @type {string} */
    this.lastLabel_ = '';
    /** @const @type {!XmltagTokenStateMachine} */
    this.stateMachine_ = new XmltagTokenStateMachine();
  }

  /**
   * @public
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
    if (this.document_.charCodeAt(end - 1) !== Unicode.GREATER_THAN_SIGN)
      return this.setSyntax(start + 2, end, 'html_element_name');
    this.setSyntax(start + 2, end - 1, 'html_element_name');
    this.setSyntax(end - 1, end, 'keyword');
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
    if (this.document_.charCodeAt(end - 1) !== Unicode.GREATER_THAN_SIGN)
      return this.paintTag(start + 1, end);
    if (this.document_.charCodeAt(end - 2) === Unicode.SOLIDUS) {
      this.setSyntax(end - 2, end, 'keyword');
      return this.paintTag(start + 1, end - 2);
    }
    this.setSyntax(end - 1, end, 'keyword');
    return this.paintTag(start + 1, end - 1);
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
   */
  paintTagName(start, end) {
    /** @const @type {string} */
    const fullName = this.document_.slice(start, end);
    if (this.keywords_.has(fullName))
      return this.setSyntax(start, end, 'keyword');
    /** @const @type {number} */
    const colonIndex = fullName.indexOf(':');
    if (colonIndex <= 1)
      return this.setSyntax(start, end, 'html_element_name');
    /** @const @type {string} */
    const prefix = fullName.substr(0, colonIndex);
    if (!this.keywords_.has(prefix))
      return this.setSyntax(start, end, 'html_element_name');
    /** @const @type {number} */
    const prefixEnd = start + end;
    this.setSyntax(start, prefixEnd, 'keyword');
    this.setSyntax(prefixEnd, end, 'html_element_name');
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
    if (this.isTagName_) {
      this.isTagName_ = false;
      this.paintTagName(start, end);
      return;
    }
    if (this.lastLabel_ === 'equal')
      return this.setSyntax(start, end, 'html_attribute_value');

    /** @const @type {string} */
    const name = this.document_.slice(start, end);
    if (this.keywords_.has(name))
      return this.setSyntax(start, end, 'keyword');
    return this.setSyntax(start, end, 'html_attribute_name');
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
    this.isTagName_ = true;
    this.lastLabel_ = '';
    this.stateMachine_.resetTo(0);
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

/** @constructor */
highlights.TagPainter = TagPainter;
});
