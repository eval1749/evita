// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('css.Property');
goog.require('testing');

goog.scope(function() {

const Property = css.Property;

testing.test('css.Property.basic', function(t) {
  t.expect(Property.nameOf(Property.idOf('color'))).toEqual('color');
  t.expect(Property.idOf('x-color')).toEqual(-1);
});

});
