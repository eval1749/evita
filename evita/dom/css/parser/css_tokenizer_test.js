// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('css.Tokenizer');
goog.require('testing');

goog.scope(function() {

const Tokenizer = css.Tokenizer;

/**
 * @param {string} source
 * @return {string}
 */
function testTokenize(source) {
  /** @type {!Array<string>} */
  const result = [];
  for (const token of(new Tokenizer()).tokensOf(source))
    result.push(token.typeString());
  return result.join(' ');
}

testing.test('css.Tokenizer.basic', function(t) {
  t.expect(testTokenize('foo')).toEqual('IDENT END');
  t.expect(testTokenize('_foo')).toEqual('IDENT END');
  t.expect(testTokenize('-foo')).toEqual('IDENT END');
  t.expect(testTokenize('--foo')).toEqual('IDENT END');
  t.expect(testTokenize('foo(')).toEqual('FUNCTION END');
  t.expect(testTokenize('123')).toEqual('NUMBER END');
  t.expect(testTokenize('12.3')).toEqual('NUMBER END');
  t.expect(testTokenize('12E3')).toEqual('NUMBER END');
  t.expect(testTokenize('12E+34')).toEqual('NUMBER END');
  t.expect(testTokenize('12E-34')).toEqual('NUMBER END');
  t.expect(testTokenize('+1')).toEqual('NUMBER END');
  t.expect(testTokenize('+1.1')).toEqual('NUMBER END');
  t.expect(testTokenize('-1')).toEqual('NUMBER END');
  t.expect(testTokenize('-1.0')).toEqual('NUMBER END');
  t.expect(testTokenize('5%')).toEqual('PERCENT END');
  t.expect(testTokenize('1pt')).toEqual('DIMENSION END');
  t.expect(testTokenize('1.2pt')).toEqual('DIMENSION END');
  t.expect(testTokenize('1.2e+3m')).toEqual('DIMENSION END');
  t.expect(testTokenize('#r2d2')).toEqual('HASH END');
  t.expect(testTokenize('@r2d2')).toEqual('AT END');
  t.expect(testTokenize('"string"')).toEqual('STRING END');
  t.expect(testTokenize("'string'")).toEqual('STRING END');
  t.expect(testTokenize('/* comment */')).toEqual('END');
  t.expect(testTokenize('+')).toEqual('DELIM END');
  t.expect(testTokenize('~')).toEqual('TILDE END');
  t.expect(testTokenize('|')).toEqual('BAR END');
  t.expect(testTokenize('(')).toEqual('LPAREN END');
  t.expect(testTokenize(')')).toEqual('RPAREN END');
  t.expect(testTokenize('[')).toEqual('LBRACKET END');
  t.expect(testTokenize(']')).toEqual('RBRACKET END');
  t.expect(testTokenize('{')).toEqual('LBRACE END');
  t.expect(testTokenize('}')).toEqual('RBRACE END');
});

testing.test('css.Tokenizer.common', function(t) {
  t.expect(testTokenize('foo { color: #123; }'))
      .toEqual('IDENT LBRACE IDENT COLON HASH SEMICOLON RBRACE END');
  t.expect(testTokenize('foo { border: red 1; }'))
      .toEqual('IDENT LBRACE IDENT COLON IDENT NUMBER SEMICOLON RBRACE END');
  t.expect(testTokenize('foo /* comment */ bar')).toEqual('IDENT IDENT END');
  t.expect(testTokenize('rgb(50%, 50%, 30%)'))
      .toEqual('FUNCTION PERCENT COMMA PERCENT COMMA PERCENT RPAREN END');
  t.expect(testTokenize('rgba(1, 2, 3, 0.1)'))
      .toEqual(
          'FUNCTION NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER RPAREN END');
});

testing.test('css.Tokenizer.delimiter', function(t) {
  t.expect(testTokenize('.hover')).toEqual('DELIM IDENT END');
  t.expect(testTokenize('&amp')).toEqual('OTHER IDENT END');
  t.expect(testTokenize('%name')).toEqual('OTHER IDENT END');
  t.expect(testTokenize('+name')).toEqual('DELIM IDENT END');
  t.expect(testTokenize('/name')).toEqual('OTHER IDENT END');
  t.expect(testTokenize(':name')).toEqual('COLON IDENT END');
  t.expect(testTokenize('<name')).toEqual('DELIM IDENT END');
  t.expect(testTokenize('=name')).toEqual('OTHER IDENT END');
  t.expect(testTokenize('>name')).toEqual('OTHER IDENT END');
  t.expect(testTokenize('?name')).toEqual('OTHER IDENT END');
  t.expect(testTokenize('~name')).toEqual('TILDE IDENT END');
});

testing.test('css.Tokenizer.error', function(t) {
  t.expect(testTokenize('/* ...'), 'unclosed comment').toEqual('INVALID END');
  t.expect(testTokenize('\'...', 'unclosed string')).toEqual('INVALID END');
  t.expect(testTokenize('"...', 'unclosed string')).toEqual('INVALID END');
});

});
