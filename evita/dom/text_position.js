// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @constructor
   * @param {!Document} document
   * @param {!number} offset
   */
  global.TextPosition = function(document, offset) {
    if (offset < 0 || offset > document.length)
      throw new RangeError('Invalid offset ' + offset + ' for ' + document);
    this.document = document;
    this.offset = offset;
  }

  /** * @return {number} */
  global.TextPosition.prototype.charCode = function() {
    return this.document.charCodeAt_(this.offset);
  }

  /** * @return {number} */
  global.TextPosition.prototype.charSyntax = function() {
    return this.document.styleAt(this.offset).charSyntax || 0;
  }

  /**
   * @this {!TextPosition}
   * @param {!Unit} unit
   * @param {number=} opt_count, default is one.
   */
  global.TextPosition.prototype.move = function(unit, opt_count) {
    var count = arguments.length >= 2 ? /** @type{number} */(opt_count) : 1;
    this.offset = this.document.computeMotion_(unit, count, this.offset);
    return this;
  }

  /**
   * @this {!TextPosition}
   * @param {function() : boolean} callback
   * @param {number=} opt_count, default is one
   * @return {!TextPosition}
   */
  global.TextPosition.prototype.moveWhile = function(callback, opt_count) {
    var count = arguments.length >= 2 ? /** @type{number} */(opt_count) : 1;
    if (count < 0) {
      while (this.offset) {
        --this.offset;
        if (!callback.call(this)) {
          ++this.offset;
          break;
        }
      }
    } else if (count > 0) {
      var end = this.document.length;
      while (this.offset < end) {
        if (!callback.call(this))
          break;
        ++this.offset;
      }
    }
    return this;
  };
})();
