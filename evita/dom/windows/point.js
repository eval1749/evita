// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {number=} opt_x
 * @param {number=} opt_y
 *
 * Note: Closure compiler doesn't allow to write |function Point|, we use
 * IIFE to set constructor name to |Point| rather than |global.Point|.
 */
global.Point = (function() {
  function Point(opt_x, opt_y) {
    this.x = opt_x || 0;
    this.y = opt_y || 0;
  }
  return Point;
})();
