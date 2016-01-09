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
 * @param {!Event} event
 * Default event handler.
 */
global.FormControl.handleEvent = function(event) {
  // Redirect event to |Form.handleEvent| if control is associated to form.
  var control = /** @type {!FormControl} */(event.target);
  var form = control.form;
  if (form) {
    Form.handleEvent.call(form, event);
    return;
  }
};
