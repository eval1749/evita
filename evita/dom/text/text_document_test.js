// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('TextDocument.replace', function(t) {
  const doc = new TextDocument();
  doc.replace(0, 0, 'abc');
  t.expect(doc.slice(0), 'insert to empty doc').toEqual('abc');

  doc.replace(3, 3, 'def');
  t.expect(doc.slice(0), 'append to end').toEqual('abcdef');

  doc.replace(2, 4, 'CD');
  t.expect(doc.slice(0), 'replace with same length').toEqual('abCDef');

  doc.replace(2, 4, '');
  t.expect(doc.slice(0), 'delete').toEqual('abef');

  doc.replace(1, 1, 'XY');
  t.expect(doc.slice(0), 'insert').toEqual('aXYbef');

  doc.replace(1, 6, 'xyz');
  t.expect(doc.slice(0), 'replace with shorter').toEqual('axyz');

  doc.replace(1, 3, '012');
  t.expect(doc.slice(0), 'replace with longer').toEqual('a012z');
});
