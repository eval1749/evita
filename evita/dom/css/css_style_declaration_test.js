// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('css.Property');
goog.require('testing');

goog.scope(function() {

const CSSStyleDeclaration = css.CSSStyleDeclaration;

testing.test('css.CSSStyleDeclaration.cssText', function(t) {
  const style = new CSSStyleDeclaration();
  style.cssText = 'color: #123; background-color: #456';
  t.expect(style.color).toEqual('#123');
  t.expect(style.backgroundColor).toEqual('#456');
  t.expect(style.cssText, 'cssText puts ";" after last property')
      .toEqual('color: #123; background-color: #456;');
});

});
