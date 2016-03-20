// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('TextMutationObserver.callback', function(t) {
  let callCount = 0;
  const observer =
      new TextMutationObserver((records, observer) => { ++callCount; });
  const document = new TextDocument();
  observer.observe(document, {summary: true});
  const range = new TextRange(document);
  range.text = 'abcdefghijklmnopqrstuvwxyz';
  range.collapseTo(11);
  range.insertBefore('----');
  testRunner.runMicrotasks();
  t.expect(callCount, 'callback should be called').toEqual(1);
  range.text = '---';
  testRunner.runMicrotasks();
  t.expect(callCount, 'callback should be called').toEqual(2);
  observer.disconnect();
  range.text = '';
  testRunner.runMicrotasks();
  t.expect(callCount, 'callback should be called').toEqual(2);
});

testing.test('TextMutationObserver.takeRecords', function(t) {
  const document = new TextDocument();
  const range = new TextRange(document);
  range.text = 'abcdefghijklmnopqrstuvwxyz';
  const observer = new TextMutationObserver(() => {});
  observer.observe(document, {summary: true});
  range.collapseTo(11);
  range.insertBefore('----');
  // "abcdefghijk----lmnopqrstuvwxyz"
  //            >....<
  range.collapseTo(20);
  range.end = 25;
  range.text = '';
  // "abcdefghijk----lmnopvwxyz"
  //            >.........<
  const records = observer.takeRecords();
  t.expect(records.length).toEqual(1);
  const actual = records[0];
  t.expect(actual instanceof TextMutationRecord).toEqual(true);
  t.expect(actual.documentLength).toEqual(26);
  t.expect(actual.headCount).toEqual(11);
  t.expect(actual.tailCount).toEqual(5);
  observer.disconnect();
});
