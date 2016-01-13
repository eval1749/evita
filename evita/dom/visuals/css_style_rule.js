// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * CSSStyleRule
 * @implements {CSSStyleObserver}
 */
class CSSStyleRule extends CSSRule {
  /**
   * @param {string} selectorText
   * @param {!CSSStyleDeclaration} style
   */
  constructor(selectorText, style) {
    super();

    /** @const @type {string} */
    this.selectorText_ = selectorText;

    /** @const @type {!CSSStyleDeclaration} */
    this.style_ = style;
  }

  /** @override @return {string} */
  get cssText() {
    return `${this.selectorText} { ${this.style_.cssText} }`;
  }

  // TODO(eval1749): NYI CSSStyleRule.prototype.cssText setter

  /** @return {string} */
  get selectorText() { return this.selectorText_; }

  // TODO(eval1749): NYI CSSStyleRule.prototype.selectorText setter

  /** @return {!CSSStyleDeclaration} */
  get style() { return this.style_; }

  /** @override @return {number} */
  get type() { return CSSRule.STYLE_RULE; }

  /**
   * CSSStyleObserver
   * @param {!CSSStyleDeclaration} style
   */
  didChangeCSSStyle(style) {
    if (!this.parentStyleSheet_)
      return;
    console.assert(this.index_ >= 0);
    this.parentStyleSheet_.didChangeCSSRuleStyle(this.index_);
  }
}
