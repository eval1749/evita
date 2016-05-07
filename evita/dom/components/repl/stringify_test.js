// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('repl');
goog.require('testing');

goog.scope(function() {
const stringify = repl.stringify;

testing.test('stringify.Array', function(t) {
  t.expect(stringify([])).toEqual('[]');
  t.expect(stringify([1])).toEqual('[1]');
  t.expect(stringify([1, 2])).toEqual('[1, 2]');
});

});
