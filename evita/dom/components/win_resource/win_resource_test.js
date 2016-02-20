// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('WinResource.load', function(t) {
  testRunner.setOpenResult('OpenWinResource', 0);
  var expectedData = new Uint8Array(3);
  expectedData[0] = 100;
  expectedData[1] = 101;
  expectedData[2] = 102;
  testRunner.setResource('icon', '#42', expectedData);
  let data = new Uint8Array(100);
  let size = 0;
  WinResource.open("foo.dll").then(
      resource => resource.load("icon", "#42", data).then(x => size = x));
  testRunner.runMicrotasks();
  t.expect(size).toEqual(3);
  t.expect(data[0]).toEqual(expectedData[0]);
  t.expect(data[1]).toEqual(expectedData[1]);
  t.expect(data[2]).toEqual(expectedData[2]);
});

testing.test('WinResource.openFailed', function(t) {
  testRunner.setOpenResult('OpenWinResource', 123);
  let reason = {winLastError: 0};
  WinResource.open("foo.dll").catch(x => reason = x);
  testRunner.runMicrotasks();
  t.expect(reason['winLastError']).toStrictEqual(123);
});
