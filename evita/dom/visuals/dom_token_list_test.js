// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('DOMTokenList', function(t) {
  class MockOwner {
    constructor() {
      this.addedTokens_ = [];
      this.removedTokens_ = [];
    }

    didAddToken(token) { this.addedTokens_.push(token); }
    didRemoveToken(token) { this.removedTokens_.push(token); }
  }

  const owner = new MockOwner();
  const list = new DOMTokenList(owner);

  t.expect(list.length).toEqual(0);

  list.add('foo');
  t.expect(list.length).toEqual(1);
  t.expect(list.item(0)).toEqual('foo');
  t.expect(owner.addedTokens_.join(' ')).toEqual('foo');

  list.add('foo');
  t.expect(list.length).toEqual(1);
  t.expect(owner.addedTokens_.join(' ')).toEqual('foo');

  list.add('bar');
  t.expect(list.length).toEqual(2);
  t.expect(owner.addedTokens_.join(' ')).toEqual('foo bar');

  list.add('baz');
  t.expect(list.length).toEqual(3);
  t.expect(owner.addedTokens_.join(' ')).toEqual('foo bar baz');

  list.remove('bar');
  t.expect(list.length).toEqual(2);
  t.expect([...list].join(' ')).toEqual('foo baz');
  t.expect(owner.removedTokens_.join(' ')).toEqual('bar');

  list.toggle('bar');
  t.expect(list.length).toEqual(3);
  t.expect([...list].join(' ')).toEqual('foo baz bar');
  t.expect(owner.addedTokens_.join(' ')).toEqual('foo bar baz bar');

  list.toggle('baz');
  t.expect(list.length).toEqual(2);
  t.expect([...list].join(' ')).toEqual('foo bar');
  t.expect(owner.removedTokens_.join(' ')).toEqual('bar baz');
});
