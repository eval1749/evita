// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('CSSStyleSheet', function(t) {
  const styleSheet = new CSSStyleSheet();
  const bodyStyle = new CSSStyleDeclaration();
  bodyStyle.color = '#080';
  const divStyle = new CSSStyleDeclaration();
  divStyle.color = '#00f';
  const spanStyle = new CSSStyleDeclaration();
  spanStyle.color = '#0f0';

  function toString(styleSheet) {
    return styleSheet.cssRules.map(rule => rule.cssText).join(' ');
  }

  styleSheet.appendRule(new CSSStyleRule('body', bodyStyle));
  t.expect(toString(styleSheet)).toEqual('body { color: #080; }');

  t.expect(styleSheet.cssRules[0]).toEqual(styleSheet.cssRules[0]);
  t.expect(styleSheet.cssRules[0].type).toEqual(CSSRule.STYLE_RULE);
  t.expect(styleSheet.cssRules[0].cssText).toEqual('body { color: #080; }');

  styleSheet.insertRule(new CSSStyleRule('div', divStyle), 0);
  t.expect(toString(styleSheet))
      .toEqual('div { color: #00f; } body { color: #080; }');

  styleSheet.insertRule(new CSSStyleRule('span', spanStyle), 1);
  t.expect(toString(styleSheet))
      .toEqual('div { color: #00f; } span { color: #0f0; } ' +
               'body { color: #080; }');

  styleSheet.deleteRule(1);
  t.expect(toString(styleSheet))
      .toEqual('div { color: #00f; } body { color: #080; }');
});
