// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @implements {CSSStyleObserver}
 * @implements {DOMTokenListOwner}
 */
class Element extends Node {
  /**
   * @param {!Document} document
   * @param {!NodeHandle} handle
   * @param {string} tagName
   * @param {string} id
   */
  constructor(document, handle, tagName, id) {
    super(document, handle);
    /** @type {DOMTokenList} */
    this.classList_ = null;
    /** @type {*} */
    this.data_ = null;
    /** @const @type {string} id */
    this.id_ = id;
    /** @type {Map<number, string>} */
    this.rawStyle_ = new Map();
    /** @type {CSSStyleDeclaration} */
    this.style_ = null;
    /** @const @type {string} id */
    this.tagName_ = tagName;
  }

  /** @return {!DOMTokenList} */
  get classList() {
    if (!this.classList_)
      this.classList_ = new DOMTokenList(this);
    return /** @type {!DOMTokenList} */(this.classList_);
  }

  /** @return {*} */
  get data() { return this.data_; }

  /** @param {*} newData */
  set data(newData) { this.data_ = newData; }

  /** @return {string} */
  get id() { return this.id_; }

  /** @return {string} */
  get tagName() { return this.tagName_; }

  /** @override @return {string} */
  get nodeName() { return this.tagName; }

  /** @return {!CSSStyleDeclaration} */
  get style() {
    this.reloadRawStyle(NodeHandle.getInlineStyle(this.handle_));
    return /** @type {!CSSStyleDeclaration} */(this.style_);
  }

  /** @param {!CSSStyleDeclaration} newStyle */
  set style(newStyle) {
    this.reloadRawStyle(newStyle.rawStyle_);
    NodeHandle.setInlineStyle(
        this.handle_,
       /** @type {!Map.<number, string>} */(this.rawStyle_));
  }

  /**
   * @private
   * @param {!Map<number, string>} newRawStyle
   */
  reloadRawStyle(newRawStyle) {
    if (!this.style_) {
      this.rawStyle_ = new Map();
      this.style_ = new CSSStyleDeclaration(this.rawStyle_, this);
    }
    this.rawStyle_.clear();
    for (const key of newRawStyle.keys())
      this.rawStyle_.set(key, newRawStyle.get(key) || '');
  }

  /** @return {string} */
  toString() {
    return `#{Element ${this.tagName_}}`;
  }

  /**
   * CSSStyleChangeObserver
   * @param {!CSSStyleDeclaration} style
   */
  didChangeCSSStyle(style) {
    NodeHandle.setInlineStyle(
        this.handle_,
        /** @type {!Map<number, string>} */(this.rawStyle_));
  }

  /**
   * DOMTokenListOwner
   * @param {string} token
   */
  didAddToken(token) {
    NodeHandle.addClass(this.handle_, token);
  }

  /**
   * DOMTokenListOwner
   * @param {string} token
   */
  didRemoveToken(token) {
    NodeHandle.removeClass(this.handle_, token);
  }
}
