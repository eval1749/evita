// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('WinRegistry.readString', function(t) {
  t.expect(WinRegistry.readString('HKEY_CLASSES_ROOT\\.txt\\'))
      .toEqual('txtfile');
});
