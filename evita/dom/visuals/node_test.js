// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('Node', function(t) {
  const document = new Document();
  const body = document.createElement('body');
  const div1 = document.createElement('div1', 'div1');
  const div2 = document.createElement('div2', 'div2');
  const div3 = document.createElement('div3', 'div3');

  t.expect(body.inDocument()).toEqual(false);

  t.expect(document.appendChild(body)).toEqual(body);
  t.expect(body.appendChild(div1)).toEqual(div1);
  t.expect(body.appendChild(div2)).toEqual(div2);

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

  // ReplaceChild
  t.expect(body.replaceChild(div3, div2)).toEqual(div2);
  t.expect(div1.parentNode).toEqual(body);
  t.expect(div1.nextSibling).toEqual(div3);
  t.expect(div1.previousSibling).toEqual(null);

  t.expect(div2.parentNode).toEqual(null);
  t.expect(div2.nextSibling).toEqual(null);
  t.expect(div2.previousSibling).toEqual(null);

  t.expect(div3.parentNode).toEqual(body);
  t.expect(div3.nextSibling).toEqual(null);
  t.expect(div3.previousSibling).toEqual(div1);

  // InsertBefore
  t.expect(body.insertBefore(div2, div3)).toEqual(div2);
  t.expect(div1.parentNode).toEqual(body);
  t.expect(div1.nextSibling).toEqual(div2);
  t.expect(div1.previousSibling).toEqual(null);

  t.expect(div2.parentNode).toEqual(body);
  t.expect(div2.nextSibling).toEqual(div3);
  t.expect(div2.previousSibling).toEqual(div1);

  t.expect(div3.parentNode).toEqual(body);
  t.expect(div3.nextSibling).toEqual(null);
  t.expect(div3.previousSibling).toEqual(div2);
});
