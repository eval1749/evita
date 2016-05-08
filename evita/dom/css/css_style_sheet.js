// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('css.CSSStyleSheet');

goog.scope(function() {

/** @constructor */
const CSSRule = css.CSSRule;

//////////////////////////////////////////////////////////////////////
//
// CSSStyleSheet
//
class CSSStyleSheet {
  constructor() {
    /** @private @type {Array<!CSSRule>} */
    this.cachedCssRules_ = null;

    /** @private @const @type {!Array<!CSSRule>} */
    this.cssRules_ = [];

    /** @const @type {!CSSStyleSheetHandle} */
    this.handle_ = new CSSStyleSheetHandle();
  }

  /** @return {!Array<!CSSRule>} */
  get cssRules() {
    if (!this.cachedCssRules_) {
      // TODO(eval17490: Once |Array.from()| has right type definition, we
      // should get rid of type case for |this.cssRules_|.
      this.cachedCssRules_ = Array.from(
          /** @type {!Iterable<!CSSRule>} */ (this.cssRules_), x => x);
    }
    return /** @type {!Array.<!CSSRule>} */ (this.cachedCssRules_);
  }

  /** @return {string} */
  get type() { return 'text/css'; }

  // TODO(eval1749) Make |CSSStyleSheet.prototype.appendRule()| to take
  // string instead of |CSSStyleRule|.
  /**
   * @param {!CSSStyleRule} rule
   */
  appendRule(rule) {
    if (rule.parentStyleSheet)
      throw new Error(`${rule} is already in ${rule.parentStyleSheet}`);
    CSSStyleSheetHandle.appendStyleRule(
        this.handle_, rule.selectorText, rule.style.rawStyle_);
    rule.parentStyleSheet_ = this;
    this.cssRules_.push(rule);
    this.clearCssRulesCache_();
  }

  /** @private */
  clearCssRulesCache_() { this.cachedCssRules_ = null; }

  /**
   * @param {number} index
   */
  deleteRule(index) {
    CSSStyleSheetHandle.deleteRule(this.handle_, index);
    this.cssRules_.splice(index, 1);
    this.clearCssRulesCache_();
  }

  /**
   * @param {number} index
   */
  didChangeCSSRule(index) {
    const rule = /** @type {!CSSStyleRule} */ (this.cssRules_[index]);
    this.deleteRule(index);
    this.insertRule(rule, index);
  }

  // TODO(eval1749) Make |CSSStyleSheet.prototype.appendRule()| to take
  // string instead of |CSSStyleRule|.
  /**
   * @param {!CSSStyleRule} rule
   * @param {number} index
   */
  insertRule(rule, index) {
    if (rule.parentStyleSheet)
      throw new Error(`${rule} is already in ${rule.parentStyleSheet}`);
    CSSStyleSheetHandle.insertStyleRule(
        this.handle_, rule.selectorText, rule.style.rawStyle_, index);
    rule.parentStyleSheet_ = this;
    this.cssRules_.splice(index, 0, rule);
    this.clearCssRulesCache_();
  }
}

/** @constructor */
css.CSSStyleSheet = CSSStyleSheet;
});

// TODO(eval1749): Export for |VisualWindow|.
/** @constructor */
var CSSStyleSheet = css.CSSStyleSheet;
