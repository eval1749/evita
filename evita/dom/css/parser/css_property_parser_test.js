// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('css.Property');
goog.require('css.PropertyParser');
goog.require('testing');

goog.scope(function() {

const Property = css.Property;
const PropertyParser = css.PropertyParser;

/**
 * @param {string} text
 * @return {string}
 */
function sample(text) {
  return [...PropertyParser.parse(text)]
      .map(propery => `${propery.name}: ${propery.value}`)
      .sort()
      .join(';');
}

testing.test('css.PropertyParser.basic', function(t) {
  t.expect(
       sample('font-family: Arial, Helvetica'),
       'font-family takes a list of font family name')
      .toEqual('font-family: Arial,Helvetica');
});

testing.test('css.PropertyParser.color', function(t) {
  t.expect(sample('color: red')).toEqual('color: red');
  t.expect(sample('color: currentcolor')).toEqual('color: currentcolor');
  t.expect(sample('color: foo')).toEqual('');
  t.expect(sample('color: rgb(1, 2, 3)')).toEqual('');
});

testing.test('css.PropertyParser.shorthand', function(t) {
  t.expect(sample('border: solid 1px red'))
      .toEqual(
          'border-bottom-color: red;border-bottom-style: solid;border-bottom-width: 1px;border-left-color: red;border-left-style: solid;border-left-width: 1px;border-right-color: red;border-right-style: solid;border-right-width: 1px;border-top-color: red;border-top-style: solid;border-top-width: 1px');
  t.expect(sample('padding: 2px'))
      .toEqual(
          "padding-bottom: 2px;padding-left: 2px;padding-right: 2px;padding-top: 2px");
});

testing.test('css.PropertyParser.text-decoration-line', function(t) {
  t.expect(sample('text-decoration-line: blink'))
      .toEqual('text-decoration-line: blink');
  t.expect(sample('text-decoration-line: line-through'))
      .toEqual('text-decoration-line: line-through');
  t.expect(sample('text-decoration-line: none'))
      .toEqual('text-decoration-line: none');
  t.expect(sample('text-decoration-line: overline'))
      .toEqual('text-decoration-line: overline');
});

});
