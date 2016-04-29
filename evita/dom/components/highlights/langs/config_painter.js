// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');

goog.scope(function() {

const Painter = highlights.Painter;

class ConfigPainter extends Painter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document); }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!Painter}
   */
  static create(document) { return new ConfigPainter(document); }
}

// Export |ConfigPainter| for testing.
/** @constructor */
highlights.ConfigPainter = ConfigPainter;
});
