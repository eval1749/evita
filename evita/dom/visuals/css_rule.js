// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//////////////////////////////////////////////////////////////////////
//
// CSSRule
//
class CSSRule {
  constructor() {
    /** @const @type {CSSRule} */
    this.parentRule_ = null;

    /** @type {CSSStyleSheet} */
    this.parentStyleSheet_ = null;
  }

  /** @return {string} */
  get cssText() { throw new Error('CSSRule.prototype.cssText'); }

  /** @return {CSSRule} */
  get parentRule() { return this.parentRule_; }

  /** @return {CSSStyleSheet} */
  get parentStyleSheet() { return this.parentStyleSheet_; }

  /** @return {number} */
  get type() { throw new Error('CSSRule.prototype.type'); }

  static get STYLE_RULE() { return 1; }
  static get CHARSET_RULE() { return 2; }
  static get IMPORT_RULE() { return 3; }
  static get MEDIA_RULE() { return 4; }
  static get FONT_FACE_RULE() { return 5; }
  static get PAGE_RULE() { return 6; }
  static get MARGIN_RULE() { return 9; }
  static get NAMESPACE_RULE() { return 10; }
}
