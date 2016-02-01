// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @this {!FormControl}
   * @return {boolean}
   */
  function canFocus() {
    return !this.disabled && !!this.form;
  }

  /**
   * @this {!FormControl}
   */
  function dispatchChangeEvent() {
    const event = new Event('change', {bubbles: true});
    this.dispatchEvent(event);
  }

  /**
   * @this {!FormControl}
   */
  function dispatchInputEvent() {
    const event = new Event('input', {bubbles: true});
    this.dispatchEvent(event);
  }

  /**
   * @param {!FormControl} control
   * @param {!Event} event
   */
  function dispatchEventWithDefaultHandler(control, event) {
    if (!control.dispatchEvent(event))
      return;
    control.constructor.handleEvent.call(control, event);
  }

  /**
   * @this {!FormControl}
   */
  function focus() {
    if (!this.canFocus()) {
      throw new Error("Failed to execute 'focus' on 'FormControl': " +
          `${this} can not have focus.`);
    }
    const oldFocused = this.form.focusControl;
    if (oldFocused === this)
      return;
    this.form.focusControl = this;
    if (oldFocused) {
      dispatchEventWithDefaultHandler(
          oldFocused,
          new FocusEvent(Event.Names.BLUR, {relatedTarget: this}));
    }
    dispatchEventWithDefaultHandler(
        this,
        new FocusEvent(Event.Names.FOCUS, {relatedTarget: oldFocused}));
  }

  Object.defineProperties(FormControl.prototype, {
    accessKey_: {value: '', writable: true},
    accessKey: {
        get: function() { return this.accessKey_; },
        set: function(key) { this.accessKey_ = key; }
    },
    canFocus: {value: canFocus, writable: true},
    dispatchChangeEvent: {value: dispatchChangeEvent},
    dispatchInputEvent: {value: dispatchInputEvent},
    focus: {value: focus}
  });

  /**
   * @this {!FormControl}
   * @param {!Event} event
   * Default event handler.
   */
  function handleEvent(event) {
    if (event.defaultPrevented)
      return;
    if (!this.form)
      return;
    Form.handleEvent.call(this.form, event);
  }

  Object.defineProperties(FormControl, {
    handleEvent: {value: handleEvent},
  });
})();
