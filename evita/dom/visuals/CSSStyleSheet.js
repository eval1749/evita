// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//////////////////////////////////////////////////////////////////////
//
// CSSStyleSheet
//
class CSSStyleSheet {
  constructor() {
    /** @type {!CSSStyleSheetHandle} */
    this.handle_ = new CSSStyleSheetHandle();
    /** @type {!Array<!CSSRule>} */
    this.cssRuleList_ = [];
  }

  /** * @return {!Array<!CSSRule>} */
  get cssRules() { return Array.from(this.cssRuleList_); }

  /** @return {string} */
  get type() { return 'text/css'; }

  /**
   * @param {!CSSRule} cssRule
   * @param {number} index
   */
  appendRule(cssRule) {
    this.insertRule(cssRule, this.cssRuleList_.length);
  }

  /**
   * @param {!CSSRule} cssRule
   * @param {number} index
   */
  insertRule(cssRule, index) {
    if (cssRule.type !== CSSRule.STYLE_RULE)
      throw `Unsupported rule ${cssRule}`;
    this.cssRuleList_.insertRule(cssRule, index);
    const cssStyleRule = /** @type {!CSSStyleRule} */(cssRule);
    this.handle_.insertRule(cssStyleRule.selectorText, cssStyleRule.rawStyle_,
                            index);
  }

  /**
   * @param {number} index
   */
  deleteRule(index) {
    this.cssRuleList_.deleteRule(index);
    this.handle_.deleteRule(index);
  }
};
