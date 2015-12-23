// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('Range.prototype.match', function(t) {
  const document = new Document();
  const range = new Range(document);
  range.text = 'foo bar baz';
  const matches = range.match(new Editor.RegExp('(b)(ar)'));
  t.expect(matches.join(',')).toEqual('bar,b,ar');
});

testing.test('Range.prototype.matches', function(t) {
  const document = new Document();
  const range = new Range(document);
  range.text = 'foo bar baz';
  t.expect(range.end).toEqual(document.length);
  testing.gtest.log('range', range.start, range.end);
  const words = [...range.matches(new Editor.RegExp('\\w+'))]
      .map(matches => document.slice(matches[0].start, matches[0].end));
  t.expect(words.join(',')).toEqual('foo,bar,baz');
});
