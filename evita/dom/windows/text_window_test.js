// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require("testing");

goog.scope(function() {

testing.test('TextWindow.prototype.zoom', function(t) {
  testing.gmock.expectCallCreateTextWindow(1);
  const sample = new TextWindow(new TextRange(new TextDocument()));
  sample.zoom = 1.5;

  t.expect(sample.zoom).toEqual(1.5);

  try {
    sample.zoom = 0;
  } catch (error) {
    t.expect(error.toString())
        .toEqual(
            "RangeError: Failed to set the 'zoom' property on 'TextWindow': " +
            "TextWindow zoom must be greater than zero.");
  }

  try {
    sample.zoom = -1;
  } catch (error) {
    t.expect(error.toString())
        .toEqual(
            "RangeError: Failed to set the 'zoom' property on 'TextWindow': " +
            "TextWindow zoom must be greater than zero.");
  }
});

});
