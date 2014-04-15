// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {number=} opt_x
 * @param {number=} opt_y
 * @param {number=} opt_width
 * @param {number=} opt_height
 *
 * Note: Closure compiler doesn't allow to write |function Rect|, we use
 * IIFE to set constructor name to |Rect| rather than |global.Rect|.
 */
global.Rect = (function() {
  function Rect(opt_x, opt_y, opt_width, opt_height) {
    this.x = opt_x || 0;
    this.y = opt_y || 0;
    this.width = opt_width || 0;
    this.height = opt_height || 0;
  }
  return Rect;
})();

Object.defineProperties(Rect.prototype, {
  bottom: {
    /** @this {!Rect} @return {number} */
    get: function() { return this.y + this.height; }
  },
  left: {
    /** @this {!Rect} @return {number} */
    get: function() { return this.x; }
  },
  leftTop: {
    /** @this {!Rect} @return {!Point} */
    get: function() { return new Point(this.x, this.y); }
  },
  right: {
    /** @this {!Rect} @return {number} */
    get: function() { return this.x + this.width; }
  },
  rightBottom: {
    /** @this {!Rect} @return {!Point} */
    get: function() { return new Point(this.right, this.bottom); }
  },
  top: {
    /** @this {!Rect} @return {number} */
    get: function() { return this.y; }
  },
});
