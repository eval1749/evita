// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {number=} x
 * @param {number=} y
 *
 * Note: Closure compiler doesn't allow to write |function Point|, we use
 * IIFE to set constructor name to |Point| rather than |global.Point|.
 */
global.Point = (function() {
  function Point(x = 0, y = 0) {
    this.x = x;
    this.y = y;
  }
  return Point;
})();
