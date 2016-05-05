// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('testing');

goog.scope(function() {

testing.test('console', function(t) {
  function consoleContents() {
    return console.document.slice(0, console.document.length);
  }

  console.clear();
  console.log();
  t.expect(consoleContents()).toEqual('\n');

  console.log('foo bar', 1, 2);
  t.expect(consoleContents()).toEqual('\nfoo bar 1 2\n');

  console.clear();
  t.expect(consoleContents()).toEqual('');

  console.document.readonly = false;
  console.document.replace(0, console.document.length, 'foo');
  console.document.readonly = true;

  console.log('bar', 'baz');
  t.expect(consoleContents(), 'log() starts at fresh line')
      .toEqual('foo\nbar baz\n');
});

});
