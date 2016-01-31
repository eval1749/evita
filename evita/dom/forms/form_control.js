// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

Object.defineProperties(global.FormControl.prototype, (function() {
  /**
   * @this {!FormControl}
   * @return {boolean}
   */
  function canFocus() {
    return !this.disabled;
  }

  /**
   * @this {!FormControl}
   */
  function focus() {
    if (!this.form)
      return;
    this.form.focusControl = this;
  }

  return {
    accessKey_: {value: '', writable: true},
    accessKey: {
        get: function() { return this.accessKey_; },
        set: function(key) { this.accessKey_ = key; }
    },
    canFocus: {value: canFocus, writable: true},
    focus: {value: focus}
  };
})());

/**
 * @this {!FormControl}
 * @param {!Event} event
 * Default event handler.
 */
global.FormControl.handleEvent = function(event) {
  if (event.defaultPrevented)
    return;
  if (!this.form)
    return;
  Form.handleEvent.call(this.form, event);
};
