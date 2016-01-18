// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('ShapeNode', function(t) {
  const document = new Document();
  const shapeData = new Uint8Array(10);
  shapeData[0] = 42;
  const shape = document.createShape(shapeData);
  t.expect(shape.data[0]).toEqual(42);
});
