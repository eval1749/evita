// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('OrderedSet', function(t) {
  const set = new base.OrderedSet(function(a, b) { return a < b; });
  for (let data = 0; data < 30; data += 5) {
    set.add(data);
  }

  t.expect(set.size).toEqual(6);
  t.expect(set.find(-1)).toEqual(null);
  t.expect(set.find(0).data).toEqual(0);
  t.expect(set.find(1)).toEqual(null);
  t.expect(set.find(5).data).toEqual(5);
  t.expect(set.find(25).data).toEqual(25);
  t.expect(set.find(26)).toEqual(null);
  t.expect(set.find(30)).toEqual(null);

  t.expect(set.lowerBound(-1).data).toEqual(0);
  t.expect(set.lowerBound(0).data).toEqual(0);
  t.expect(set.lowerBound(1).data).toEqual(5);
  t.expect(set.lowerBound(5).data).toEqual(5);
  t.expect(set.lowerBound(25).data).toEqual(25);
  t.expect(set.lowerBound(26)).toEqual(null);
  t.expect(set.lowerBound(30)).toEqual(null);

  t.expect(set.minimum).toEqual(0);
  t.expect(set.maximum).toEqual(25);

  const forEachResult = [];
  set.forEach(data => forEachResult.push(data));
  t.expect(forEachResult.join(' ')).toEqual('0 5 10 15 20 25');

  t.expect([...set.entries()].join(' ')).toEqual('0 5 10 15 20 25');

  t.expect(set.remove(-1)).toEqual(false);
  t.expect(set.remove(1)).toEqual(false);
  t.expect(set.remove(11)).toEqual(false);
  t.expect(set.remove(31)).toEqual(false);
  t.expect(set.remove(10)).toEqual(true);
  t.expect(set.size).toEqual(5);
  t.expect(set.find(10)).toEqual(null);
  t.expect(set.lowerBound(10).data).toEqual(15);

  set.remove(0);
  t.expect(set.minimum).toEqual(5);

  set.remove(25);
  t.expect(set.maximum).toEqual(20);

  set.clear();
  t.expect(set.size).toEqual(0);
});
