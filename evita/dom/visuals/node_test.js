// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('Node', function(t) {
  const document = new Document();
  const body = document.createElement('body');
  const div1 = document.createElement('div', 'div1');
  const div2 = document.createElement('div', 'div2');

  t.expect(body.inDocument()).toEqual(false);

  document.appendChild(body);
  body.appendChild(div1);
  body.appendChild(div2);

  t.expect(body.inDocument()).toEqual(true);
  t.expect(div1.inDocument()).toEqual(true);
  t.expect(div2.inDocument()).toEqual(true);

  t.expect(body.isDescendantOf(div1)).toEqual(false);
  t.expect(div1.isDescendantOf(body)).toEqual(true);
  t.expect(div2.isDescendantOf(body)).toEqual(true);

  t.expect(document.firstChild).toEqual(body);
  t.expect(document.lastChild).toEqual(body);
  t.expect(body.firstChild).toEqual(div1);
  t.expect(body.lastChild).toEqual(div2);

  t.expect([...body.childNodes].map(child => child.id).join(' '))
      .toEqual('div1 div2');
});
