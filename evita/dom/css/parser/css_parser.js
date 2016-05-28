// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('css.Parser');

goog.require('base.Logger');
goog.require('css.CSSStyleDeclaration');
goog.require('css.CSSStyleRule');
goog.require('css.CSSStyleSheet');
goog.require('css.PropertyBuilder');
goog.require('css.Tokenizer');
goog.require('css.Tokenizer');

goog.scope(function() {

/** @constructor */
const Property = css.Property;

/** @constructor */
const PropertyBuilder = css.PropertyBuilder;

/** @constructor */
const Token = css.Token;

/** @constructor */
const Tokenizer = css.Tokenizer;

class RuleBuilder extends base.Logger {
  constructor() {
    super();
    /** @const @type {!Set<string>} */
    this.properties_ = new Set();
    /** @type {string} */
    this.selectorText_ = '';
    /** @type {!css.CSSStyleDeclaration} */
    this.style_ = new css.CSSStyleDeclaration();
  }

  /** @param {!Array<!Property>} properties */
  addProperties(properties) {
    for (const property of properties)
      this.addProperty(property);
  }

  /** @param {!Property} property */
  addProperty(property) {
    if (property.id < 0)
      return;
    if (this.properties_.has(property.name))
      return;
    this.log(0, 'addProperty', property.name, property.value);
    this.style_.setProperty(property.name, property.value);
    this.properties_.add(property.name);
  }

  /**
   * @return {?css.CSSStyleRule}
   */
  end() {
    console.assert(this.selectorText_ !== '');
    /** @const @type {string} */
    const selectorText = this.selectorText_;
    this.selectorText_ = '';
    if (this.properties_.size === 0)
      return null;
    /** @const @type {!css.CSSStyleRule} */
    const rule = new css.CSSStyleRule(selectorText, this.style_);
    return rule;
  }

  /**
   * @param {string} selectorText
   */
  start(selectorText) {
    console.assert(this.selectorText_ === '', this.selectorText_);
    this.selectorText_ = selectorText;
    this.properties_.clear();
    this.style_ = new css.CSSStyleDeclaration();
  }
}

class SelectorBuilder {
  constructor() {
    /** @type {!Array<!Token>} */
    this.tokens_ = [];
  }

  /** @param {!Token} token */
  add(token) {
    if (token.type === Token.Type.INVALID)
      return;
    this.tokens_.push(token);
  }

  /** @return {string} */
  end() {
    console.assert(this.tokens_.length > 0);
    /** @const @type {string} */
    const result = this.tokens_.map(token => token.text).join('');
    this.tokens_ = [];
    return result;
  }

  /** @param {!Token} token */
  start(token) { this.tokens_ = [token]; }
}

class SheetBuilder {
  constructor() {
    /** @const @type {!css.CSSStyleSheet} */
    this.sheet_ = new css.CSSStyleSheet();
  }

  /** @param {?css.CSSStyleRule} rule */
  add(rule) {
    if (!rule)
      return;
    this.sheet_.appendRule(rule);
  }

  /** @return {!css.CSSStyleSheet} */
  end() { return this.sheet_; }
}

class Parser extends base.Logger {
  constructor() { super(); }

  /**
   * @public
   * @param {string} source
   * @return {!css.CSSStyleSheet}
   */
  parse(source) {
    /** @const @type {!Tokenizer} */
    const tokenizer = new Tokenizer();
    /** @const @type {!SheetBuilder} */
    const sheetBuilder = new SheetBuilder();
    /** @type {string} */
    let state = 'start';
    /** @type {!PropertyBuilder} */
    const propertyBuilder = new PropertyBuilder();
    propertyBuilder.verbose = this.verbose;
    /** @type {!RuleBuilder} */
    const ruleBuilder = new RuleBuilder();
    ruleBuilder.verbose = this.verbose;
    /** @type {!SelectorBuilder} */
    const selectorBuilder = new SelectorBuilder();

    for (const token of tokenizer.tokensOf(source)) {
      this.log(0, state, token);
      if (token.type === Token.Type.END)
        return sheetBuilder.end();
      if (token.type === Token.Type.INVALID || token.type === Token.Type.OTHER)
        continue;
      switch (state) {
        case 'start':
          if (token.type === Token.Type.ASTERISK ||
              token.type === Token.Type.IDENT ||
              token.type === Token.Type.COLON ||
              token.type === Token.Type.DCOLON ||
              token.type === Token.Type.DOT) {
            state = 'selector';
            selectorBuilder.start(token);
            continue;
          }
          this.log(0, 'ignore', token);
          continue;
        case 'properties':
          if (token.type === Token.Type.RBRACE) {
            sheetBuilder.add(ruleBuilder.end());
            state = 'start';
            continue;
          }
          if (token.type !== Token.Type.IDENT) {
            state = 'bad properties';
            continue;
          }
          propertyBuilder.start(token);
          state = 'property_name';
          continue;
        case 'property_name':
          if (token.type === Token.Type.RBRACE) {
            sheetBuilder.add(ruleBuilder.end());
            state = 'start';
            continue;
          }
          if (token.type !== Token.Type.COLON) {
            state = 'properties';
            continue;
          }
          state = 'property_colon';
          continue;
        case 'property_colon':
          if (token.type === Token.Type.RBRACE) {
            sheetBuilder.add(ruleBuilder.end());
            state = 'start';
            continue;
          }
          if (token.type === Token.SEMICOLON) {
            this.warn('No property value');
            state = 'properties';
            continue;
          }
          propertyBuilder.add(token);
          state = 'property_value';
          continue;
        case 'property_value':
          if (token.type === Token.Type.RBRACE) {
            ruleBuilder.addProperties(propertyBuilder.end());
            sheetBuilder.add(ruleBuilder.end());
            state = 'start';
            continue;
          }
          if (token.type === Token.Type.SEMICOLON) {
            ruleBuilder.addProperties(propertyBuilder.end());
            state = 'properties';
            continue;
          }
          propertyBuilder.add(token);
          state = 'property_value';
          continue;
        case 'selector':
          if (token.type === Token.Type.LBRACE) {
            state = 'properties';
            ruleBuilder.start(selectorBuilder.end());
            continue;
          }
          if (token.type === Token.Type.RBRACE) {
            state = 'start';
            continue;
          }
          selectorBuilder.add(token);
          continue;
        case 'bad properties':
          // Skip until right brace
          if (token.type === Token.Type.RBRACE) {
            state = 'start';
            continue;
          }
          continue;
      }
      throw new Error(`Unknown state '${state}'`);
    }
    throw new Error('NOTREACEHD');
  }

  /**
   * @private
   * @param {string} message
   */
  warn(message) { this.log(0, 'WARN:', message); }

  /**
   * @public
   * @param {string} source
   * @return {!css.CSSStyleSheet}
   */
  static parse(source) { return (new Parser()).parse(source); }
}

/** @constructor */
css.Parser = Parser;

});
