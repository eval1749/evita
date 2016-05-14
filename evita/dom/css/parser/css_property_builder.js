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
const kColorStyleWidth = ['color', 'style', 'width'];

/** @const @type {!Array<string>} */
const kSidePrefixes = ['-bottom', '-left', '-right', '-top'];

/** @const @type {!Map<string, !Array<string>>} */
const kShorthandMap = new Map();
kShorthandMap.set('border-bottom', kColorStyleWidth);
kShorthandMap.set('border-left', kColorStyleWidth);
kShorthandMap.set('border-right', kColorStyleWidth);
kShorthandMap.set('border-top', kColorStyleWidth);
kShorthandMap.set('text-decoration', ['color', 'line', 'style']);

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
    console.assert(this.values_.length > 0, this.name_);
    /** @const @type {!Token} */
    const name = this.name_;
    /** @const @type {!Array<!Token>} */
    const values = this.values_;
    this.name_ = kDummyToken;
    this.values_ = [];
    switch (name.text) {
      case 'border': {
        /** @const @type {!Array<!Property>} */
        const result = [];
        for (const suffix of kSidePrefixes) {
          /** const @type {string} */
          const propertyName = `${name.text}${suffix}`;
          /** const @type {!Array<!Property>} */
          const properties =
              this.handleShorthand(propertyName, values, kColorStyleWidth);
          for (const property of properties)
            result.push(property);
        }
        return result;
      }
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
    /** @type {!Array<string>} */
    const types = kShorthandMap.get(name.text) || [];
    if (types.length > 0)
      return this.handleShorthand(name.text, values, types);
    /** @const @type {string} */
    const propertyName = name.text;
    /** @const @type {string} */
    const propertyValue = values[0].text;
    if (!Value.isValidFor(propertyValue, propertyName))
      return [];
    return [new Property(propertyName, propertyValue)];
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
   * @private
   * @param {string} name
   * @param {!Array<!Token>} values
   * @param {!Array<!string>} types
   * @return {!Array<!Property>}
   */
  handleShorthand(name, values, types) {
    /** @type {!Array<!Property>} */
    const result = [];
    /** @const @type {!Set<string>} */
    const typeSet = new Set(types);
    for (const value of values) {
      for (const type of typeSet) {
        /** @const @type {string} */
        const propertyName = `${name}-${type}`;
        /** @const @type {boolean} */
        const isValidValue = Value.isValidFor(value.text, propertyName);
        this.log(1, 'handleShorthand', propertyName, value, isValidValue);
        if (!isValidValue)
          continue;
        result.push(new Property(propertyName, value.text));
        typeSet.delete(type);
        break;
      }
    }
    return result;
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
