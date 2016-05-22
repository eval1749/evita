// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('css.CSSStyleSheet');
goog.require('css.Parser');
goog.require('testing');

goog.scope(function() {

const CSSStyleSheet = css.CSSStyleSheet;
const Parser = css.Parser;

/**
 * @param {!CSSStyleSheet} styleSheet
 * @param {string} selectorText
 * @return {string}
 */
function querySelector(styleSheet, selectorText) {
  for (const rule of styleSheet.cssRules) {
    if (rule.selectorText === selectorText)
      return rule.cssText;
  }
  return '';
}

testing.test('css.Parser.basic', function(t) {
  /** @const @type {!CSSStyleSheet} */
  const styleSheet = Parser.parse(
      'foo {color: red} bar {color: blue} /* baz {color: green} */');
  t.expect(styleSheet.cssRules.length, "comment doesn't affect").toEqual(2);
  t.expect(querySelector(styleSheet, 'foo')).toEqual('foo { color: red; }');
  t.expect(querySelector(styleSheet, 'bar')).toEqual('bar { color: blue; }');
});

testing.test('css.Parser.classSelector', function(t) {
  /** @const @type {!CSSStyleSheet} */
  const styleSheet = Parser.parse('.hover { color: blue}');
  t.expect(styleSheet.cssRules.length).toEqual(1);
  t.expect(querySelector(styleSheet, '.hover'))
      .toEqual('.hover { color: blue; }');
});

testing.test('css.Parser.empty', function(t) {
  /** @const @type {!CSSStyleSheet} */
  const styleSheet = Parser.parse(
      'foo {} ' +
      'foo {color: red;}');
  t.expect(styleSheet.cssRules.length, "Empty style doesn't in style sheet")
      .toEqual(1);
  t.expect(querySelector(styleSheet, 'foo')).toEqual('foo { color: red; }');
});

testing.test('css.Parser.notMerge', function(t) {
  /** @const @type {!CSSStyleSheet} */
  const styleSheet = Parser.parse(
      'foo {color: red} ' +
      'foo {color: blue; background-color: green}');
  t.expect(styleSheet.cssRules.length, "Selectors aren't merged").toEqual(2);
  t.expect(querySelector(styleSheet, 'foo')).toEqual('foo { color: red; }');
});

testing.test('css.Parser.pseudoClass', function(t) {
  /** @const @type {!CSSStyleSheet} */
  const styleSheet = Parser.parse(':hover { color: blue}');
  t.expect(styleSheet.cssRules.length).toEqual(1);
  t.expect(querySelector(styleSheet, ':hover'))
      .toEqual(':hover { color: blue; }');
});

testing.test('css.Parser.pseudoElement', function(t) {
  /** @const @type {!CSSStyleSheet} */
  const styleSheet = Parser.parse('div ::first-letter { color: blue}');
  t.expect(styleSheet.cssRules.length).toEqual(1);
  t.expect(querySelector(styleSheet, 'div::first-letter'))
      .toEqual('div::first-letter { color: blue; }');
});

testing.test('css.Parser.universalTypeSelector', function(t) {
  /** @const @type {!CSSStyleSheet} */
  const styleSheet = Parser.parse('* { color: red }');
  t.expect(styleSheet.cssRules.length).toEqual(1);
  t.expect(querySelector(styleSheet, '*')).toEqual('* { color: red; }');
});

});
