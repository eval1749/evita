// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('css.Value');
goog.require('testing');

goog.scope(function() {

const Value = css.Value;

// See also "//evita/visuals/css/properties.in" for implemented CSS properties
// and keywords.
testing.test('css.Value.basic', function(t) {
  t.expect(Value.isValidFor('solid', 'border-top-style')).toEqual(true);
  t.expect(Value.isValidFor('thick', 'border-top-width')).toEqual(true);
  t.expect(Value.isValidFor('foo', 'color')).toEqual(false);
  t.expect(Value.isValidFor('#123', 'color'), 'rgb').toEqual(true);
  t.expect(Value.isValidFor('#1234', 'color'), 'rgba').toEqual(true);
  t.expect(Value.isValidFor('#112233', 'color'), 'rgb').toEqual(true);
  t.expect(Value.isValidFor('#11223344', 'color'), 'rgba').toEqual(true);
  t.expect(Value.isValidFor('red', 'color')).toEqual(true);
  t.expect(Value.isValidFor('currentcolor', 'color')).toEqual(true);
  t.expect(Value.isValidFor('transparent', 'color')).toEqual(true);
});

});
