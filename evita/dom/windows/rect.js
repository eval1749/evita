// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {number=} x
 * @param {number=} y
 * @param {number=} width
 * @param {number=} height
 *
 * Note: Closure compiler doesn't allow to write |function Rect|, we use
 * IIFE to set constructor name to |Rect| rather than |global.Rect|.
 */
global.Rect = (function() {
  function Rect(x = 0, y = 0, width = 0, height = 0) {
    this.x = x;
    this.y = y;
    this.width = width;
    this.height = height;
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
