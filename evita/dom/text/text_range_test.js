// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('testing');

goog.scope(function() {

/**
 * @param {!TextDocument} document
 * @return {string}
 */
function spellingMarkersOf(document) {
  /** @const @type {!Array<string>} */
  const result = [];
  for (let offset = 0; offset < document.length; ++offset)
    result.push((document.spellingAt(offset) + '.').substr(0, 1));
  return result.join('');
}

testing.test('TextRange.prototype.match', function(t) {
  const document = new TextDocument();
  const range = new TextRange(document);
  range.text = 'foo bar baz';
  const matches = range.match(new Editor.RegExp('(b)(ar)'));
  t.expect(matches.join(',')).toEqual('bar,b,ar');
});

testing.test('TextRange.prototype.matches', function(t) {
  const document = new TextDocument();
  const range = new TextRange(document);
  range.text = 'foo bar baz';
  t.expect(range.end).toEqual(document.length);
  testing.gtest.log('range', range.start, range.end);
  const words = [...range.matches(new Editor.RegExp('\\w+'))].map(
      matches => document.slice(matches[0].start, matches[0].end));
  t.expect(words.join(',')).toEqual('foo,bar,baz');
});

testing.test('TextRange.setSpelling', function(t) {
  const doc = new TextDocument();
  doc.replace(0, 0, 'foo bar baz');
  const range = new TextRange(doc, 4, 7);
  range.setSpelling(2);

  t.expect(spellingMarkersOf(doc)).toEqual('....mmm....');
});


});
