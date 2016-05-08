// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('css.PropertyBuilder');

goog.require('base.Logger');
goog.require('css.Property');
goog.require('css.PropertyBuilder');
goog.require('css.Token');
goog.require('css.Value');

goog.scope(function() {

/** @constructor */
const Property = css.Property;

/** @constructor */
const Token = css.Token;

/** @constructor */
const Tokenizer = css.Tokenizer;

/** @constructor */
const Value = css.Value;

/** @const @type {!Token} */
const kDummyToken = new Token(Token.Type.INVALID, '', 0, 0);

/** @const @type {!Array<string>} */
const kSidePrefixes = ['-bottom', '-left', '-right', '-top'];

// TODO(eval1749): Support function, e.g. rgba(1, 2, 3)
class PropertyBuilder extends base.Logger {
  /** @public */
  constructor() {
    super();
    /** @type {!Token} */
    this.name_ = kDummyToken;
    /** @type {!Array<!Token>} */
    this.values_ = []
  }

  /**
   * @public
   * @param {!Token} token
   */
  add(token) {
    console.assert(this.name_ !== kDummyToken, this);
    console.assert(token.type !== Token.Type.SEMICOLON, token);
    this.values_.push(token);
  }

  /**
   * @public
   * @return {!Array<!Property>}
   */
  end() {
    /** @const @type {!Token} */
    const name = this.name_;
    /** @const @type {!Array<!Token>} */
    const values = this.values_;
    this.name_ = kDummyToken;
    this.values_ = [];
    // TODO(eval1749): Support more shorthand property
    switch (name.text) {
      case 'border':
        return this.handleBorder(name, values, kSidePrefixes);
      case 'border-bottom':
      case 'border-left':
      case 'border-right':
      case 'border-top':
        return this.handleBorder(name, values, ['']);
      case 'border-color':
      case 'border-style':
      case 'border-width':
      case 'margin':
      case 'padding':
        return this.handleRepeat1to4(name, values);
      case 'font-family':
        return [new Property(
            name.text, values.map(token => token.text).join(''))];
    }
    return [new Property(name.text, values[0].text)];
  }

  /**
   * @private
   * @param {!Token} name
   * @param {!Array<!Token>} values
   * @param {!Array<string>} suffixes
   * @return {!Array<!Property>}
   *
   * Syntax:
   *  border: <line-width> || <line-style> || <color>
   *  border-top: <line-width> || <line-style> || <color>
   *  border-right: <line-width> || <line-style> || <color>
   *  border-bottom: <line-width> || <line-style> || <color>
   *  border-left: <line-width> || <line-style> || <color>
   */
  handleBorder(name, values, suffixes) {
    /** @const @type {!Map<string, string>} */
    const valueMap = new Map();
    /** @const @type {!Set<string>} */
    const typeSet = new Set(['color', 'style', 'width']);
    for (const value of values) {
      for (const type of typeSet) {
        this.log(
            0, name.text, value.text, type,
            Value.isValidFor(value.text, `border-top-${type}`));
        if (Value.isValidFor(value.text, `border-top-${type}`)) {
          if (!valueMap.has(type))
            valueMap.set(type, value.text);
          typeSet.delete(type);
          break;
        }
      }
    }
    /** @type {!Array<!Property>} */
    const result = [];
    for (const[type, value] of valueMap) {
      for (const suffix of suffixes)
        result.push(new Property(`${name.text}${suffix}-${type}`, value));
    }
    return result;
  }

  /**
   * @private
   * @param {!Token} name
   * @param {!Array<!Token>} values
   * @return {!Array<!Property>}
   *
   *  border-color: <line-width>{1,4}
   *  border-style: <line-width>{1,4}
   *  border-width: <line-width>{1,4}
   *
   *  <line-width> = <length> | thin | medium | thick
   */
  handleRepeat1to4(name, values) {
    this.log(0, name, values);
    switch (values.length) {
      case 1:
        return this.handleRepeat1to4(name, [values[0], values[0]]);
      case 2:
        return this.handleRepeat1to4(
            name, [values[0], values[1], values[0], values[1]]);
      case 4:
        return [
          new Property(`${name.text}-top`, values[0].text),
          new Property(`${name.text}-right`, values[1].text),
          new Property(`${name.text}-bottom`, values[2].text),
          new Property(`${name.text}-left`, values[3].text),
        ];
    }
    return [];
  }

  /**
   * @public
   * @param {!Token} token
   */
  start(token) {
    console.assert(this.name_ === kDummyToken, this.name_);
    this.name_ = token;
    this.values_ = [];
  }
}

/** @constructor */
css.PropertyBuilder = PropertyBuilder;

});
