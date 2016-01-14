// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//////////////////////////////////////////////////////////////////////
//
// CSSRuleBuilder
//
class CSSRuleBuilder {
  /**
   * @param {string} selectorText
   */
  constructor(selectorText) {
    /** @private @const @type {string} */
    this.selectorText_ = selectorText;

    /** @private @const @type {!CSSStyleDeclaration} */
    this.style_ = new CSSStyleDeclaration();
  }

  /**
   * @param {string} color
   * $return {!CSSRuleBuilder}
   */
  backgroundColor(color) {
    this.style_.backgroundColor = color;
    return this;
  }

  /**
   * @param {string} color
   * @param {...number} widths
   * $return {!CSSRuleBuilder}
   */
  border(color, ...widths) {
    switch (widths.length) {
      case 0:
        return this.border(color, 1);
      case 1:
        return this.border(color, widths[0], widths[0]);
      case 2:
        return this.border(color, widths[0], widths[1], widths[0], widths[1]);
      case 4:
        this.style_.borderBottomColor = color;
        this.style_.borderLeftColor = color;
        this.style_.borderRightColor = color;
        this.style_.borderTopColor = color;

        this.style_.borderTopWidth = widths[0].toString();
        this.style_.borderRightWidth = widths[1].toString();
        this.style_.borderBottomWidth = widths[2].toString();
        this.style_.borderLeftWidth = widths[3].toString();
        break;
      default:
        throw new Error(`invalid syntax for border ${color} ${widths}`);
    }
    return this;
  }

  /** @return {!CSSStyleRule} */
  build() {
    return new CSSStyleRule(this.selectorText_, this.style_);
  }

  /**
   * @param {string} display
   * $return {!CSSRuleBuilder}
   */
  display(display) {
    this.style_.display = display;
    return this;
  }

  /**
   * @param {number} size
   * $return {!CSSRuleBuilder}
   */
  fontSize(size) {
    this.style_.fontSize = size.toString();
    return this;
  }

  /**
   * @param {number} width
   * $return {!CSSRuleBuilder}
   */
  padding(width) {
    const width_string = width.toString();
    this.style_.paddingBottom = width_string;
    this.style_.paddingLeft = width_string;
    this.style_.paddingRight = width_string;
    this.style_.paddingTop = width_string;
    return this;
  }

  /**
   * @param {...number} widths
   * $return {!CSSRuleBuilder}
   */
  margin(...widths) {
    switch (widths.length) {
      case 1:
        return this.margin(widths[0], widths[0]);
      case 2:
        return this.margin(widths[0], widths[1], widths[0], widths[1]);
      case 4:
        this.style_.marginTop = widths[0].toString();
        this.style_.marginRight = widths[1].toString();
        this.style_.marginBottom = widths[2].toString();
        this.style_.marginLeft = widths[3].toString();
        break;
      default:
        throw new Error(`invalid syntax for margin ${widths}`);
    }
    return this;
  }

  /**
   * @param {number} width
   * $return {!CSSRuleBuilder}
   */
  width(width) {
    this.style_.width = width.toString();
  }

  /**
   * @param {string} selectorText
   * $return {!CSSRuleBuilder}
   */
  static selector(selectorText) {
    return new CSSRuleBuilder(selectorText);
  }
}
