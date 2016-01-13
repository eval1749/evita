// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('Node', function(t) {
  const document = new Document();
  const element = document.createElement('div', 'foo');

  t.expect([...element.classList].join(' ')).toEqual('');
  t.expect(element.id).toEqual('foo');
  t.expect(element.nodeName).toEqual('div');
  t.expect(element.tagName).toEqual('div');

  element.classList.add('c1');
  t.expect([...element.classList].join(' ')).toEqual('c1');

  element.classList.add('c2');
  t.expect([...element.classList].join(' ')).toEqual('c1 c2');

  element.classList.toggle('c1');
  t.expect([...element.classList].join(' ')).toEqual('c2');

  element.classList.toggle('c3');
  t.expect([...element.classList].join(' ')).toEqual('c2 c3');

  element.style.backgroundColor = '#fff';
  element.style.color = '#0f0';
  t.expect(element.style.cssText).toEqual(
      'background_color: #FFFFFF; color: #00FF00;');
});
