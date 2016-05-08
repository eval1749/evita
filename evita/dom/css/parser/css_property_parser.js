// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('css.PropertyParser');

goog.require('base.Logger');
goog.require('css.Property');
goog.require('css.PropertyBuilder');
goog.require('css.Token');
goog.require('css.Tokenizer');
goog.require('css.Value');

goog.scope(function() {

/** @constructor */
const Property = css.Property;
/** @constructor */
const PropertyBuilder = css.PropertyBuilder;
/** @constructor */
const Token = css.Token;
/** @constructor */
const Tokenizer = css.Tokenizer;

class PropertyParser extends base.Logger {
  constructor() { super(); }

  /**
   * @public
   * @param {string} text
   * @return {!Generator<!Property>}
   */
  * parse(text) {
    /** @const @type {!Tokenizer} */
    const tokenizer = new Tokenizer();
    tokenizer.verbose = this.verbose;
    /** @type {!PropertyBuilder} */
    const propertyBuilder = new PropertyBuilder();
    propertyBuilder.verbose = this.verbose;
    /** @type {string} */
    let state = 'start';
    for (const token of tokenizer.tokensOf(text)) {
      this.log(0, state, token);
      if (token.type === Token.Type.END)
        break;
      switch (state) {
        case 'name':
          if (token.type === Token.Type.COLON)
            state = 'value';
          continue;
          if (token.type === Token.Type.IDENT) {
            propertyBuilder.start(token);
            continue;
          }
          state = 'start';
          continue;
        case 'start':
          if (token.type !== Token.Type.IDENT)
            continue;
          propertyBuilder.start(token);
          state = 'name';
          continue;
        case 'value':
          if (token.type === Token.Type.INVALID)
            continue;
          if (token.type !== Token.Type.SEMICOLON) {
            propertyBuilder.add(token);
            continue;
          }
          for (const property of propertyBuilder.end())
            yield property;
          state = 'start';
          continue;
      }
      throw new Error(`NOTREACEHD state='${state}'`);
    }
    if (state !== 'value')
      return;
    for (const property of propertyBuilder.end())
      yield property;
  }

  /**
   * @public
   * @param {string} text
   * @return {!Generator<!Property>}
   */
  static * parse(text) { yield * (new PropertyParser()).parse(text); }
}

/** @constructor */
css.PropertyParser = PropertyParser;
});
