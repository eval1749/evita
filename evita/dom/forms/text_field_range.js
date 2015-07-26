// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @type {!TextFieldControl} */
TextFieldRange.prototype.control_;

/** @type {number} */
TextFieldRange.prototype.end_;

/** @type {number} */
TextFieldRange.prototype.start_;

/**
 * @constructor
 * @param {!TextFieldControl} control
 */
global.TextFieldRange = function(control) {
  this.control_ = control;
  if (!control.ranges_) {
    // TODO(yosi) Until |WeakSet| supports |forEach| method, we use
    // to sue |WeakMap|.
    control.ranges_ = new WeakMap();
    control.ranges_['size_'] = 0;
  }
  var size = control['size_'];
  control.ranges_.add(size, this);
  control.ranges_['size_'] = size + 1;
};

Object.defineProperties(TextFieldRange.prototype, {
  collapsed: {get:
    /** @this {!TextFieldRange} @return {boolean} */ 
    function() { return this.end_ == this.start_; }
  },
  control_: {value: null, writable: true },
  control: { get:
    /** @this {!TextFieldRange} @return {!TextFieldControl} */
    function() { return this.control_;}
  },
  end_: {value: 0, writable: true},
  end: {
    get: /** @this {!TextFieldRange} @return {number} */
      function() { return this.end_; },
    /**
     * @this {!TextFieldRange}
     * @param {number} new_end
     */
    set: function(new_end) {
      if (this.start_ < new_end)
        this.start_ = new_end;
      this.end_ = new_end;
    }
  },
  start_: {value: 0, writable: true},
  start: {
    get:  /** @this {!TextFieldRange} @return {number} */
      function() { return this.start_; },
    /**
     * @this {!TextFieldRange}
     * @param {number} new_start
     */
    set: function(new_start) {
      if (this.end_ < new_start)
        this.end_ = new_start;
      this.start_ = new_start;
    }
  },
  text: {
    get:
      /**
       * @this {!TextFieldRange}
       * @return {string}
       */
      function() {
        return this.collapsed ? '' :
            this.control_.value.substring(this.start_, this.end_);
      },
    set:
      /**
       * @this {!TextFieldRange}
       * @param {string} new_text
       */
      function(new_text) {
        var start = this.start_;
        this.control_.textBuffer.replace(start, this.end_, new_text);
        this.start_ = start;
        this.end_ = start + new_text.length;
      }
  },
});
