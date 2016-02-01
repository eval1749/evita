// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @param {!Form} form
   * @param {FormControl} control
   * @return {number}
   */
  function controlIndex(form, control) {
    if (!control)
      return -1;
    const controls = form.controls;
    for (let i = 0; i < controls.length; ++i) {
      if (controls[i] === control)
        return i;
    }
    return -1;
  }

  /**
   * Move focus to next focusable control in |form|. If no control has focus,
   * we set focus to first focusable control. Order of control is as same as
   * index of control in |Form.prototype.controls| vector.
   * @param {!Form} form
   */
  function moveFocusToNext(form) {
    /** @type {!Array.<!FormControl>} */
    const controls = form.controls;
    function findFocusable(start) {
      for (let i = start + 1; i < controls.length; ++i) {
        const control = controls[i];
        if (control.canFocus())
          return control;
      }
      return null;
    }
    /** @type {FormControl} */
    const focusControl = findFocusable(controlIndex(form, form.focusControl)) ||
                         findFocusable(-1);
    if (!focusControl)
      return;
    focusControl.focus();
  }

  /**
   * Move focus to previous focusable control in |form|. If no control has
   * focus, we set focus to first focusable control. Order of control is as
   * same as index of control in |Form.prototype.controls| vector.
   * @param {!Form} form
   */
  function moveFocusToPrevious(form) {
    /** @type {!Array.<!FormControl>} */
    const controls = form.controls;
    function findFocusable(start) {
      for (let i = start - 1; i >= 0; --i) {
        const control = controls[i];
        if (control.canFocus())
          return control;
      }
      return null;
    }
    /** @type {FormControl} */
    const focusControl = findFocusable(controlIndex(form, form.focusControl)) ||
                         findFocusable(controls.length);
    if (!focusControl)
      return;
    focusControl.focus();
  }

  /**
   * @param {!Form} form
   * @param {!KeyboardEvent} event
   *
   * Set focus to control which have matched access key for controls other
   * than button, checkbox and radio button:
   *  * button: dispatch "click" event.
   *  * checkbox: toggle |checked| property.
   *  * radio button: toggle |checked| property.
   */
  function handleAltKeyDown(form, event) {
    /** @type {string} */
    const accessKey = String.fromCharCode(event.keyCode & 0x1FF);
    /** @type {FormControl} */
    const control = form.controls.find((control) => {
      return control.accessKey === accessKey;
    }) || null;
    if (!control || control.disabled)
      return;
    if (control instanceof ButtonControl) {
      control.dispatchEvent(new MouseEvent(Event.Names.CLICK));
      return;
    }
    control.focus();
    if (control instanceof CheckboxControl) {
      control.checked = !control.checked;
      return;
    }
    if (control instanceof RadioButtonControl) {
      control.checked = !control.checked;
      return;
    }
  }

  /**
   * @param {!Form} form
   * @param {!KeyboardEvent} event
   */
  function handleKeyDown(form, event) {
    if (event.ctrlKey)
      return;
    if (event.altKey)
      return handleAltKeyDown(form, event);
    // Move focus to next/previous of current focus control.
    switch (event.keyCode & 0x1FF) {
      case 0x109: // TAB
        if (event.shiftKey)
          moveFocusToPrevious(form);
        else
          moveFocusToNext(form);
        break;
      case 0x125: // Left
      case 0x126: // Up
        moveFocusToPrevious(form);
        break;
      case 0x127: // ArrowRight
      case 0x128: // ArrowDown
        moveFocusToNext(form);
        break;
    }
  }

  /**
   * @this {!Form}
   * @param {!Event} event
   * Default event handler.
   */
  function handleEvent(event) {
    switch (event.type) {
      case Event.Names.BLUR:
        if (event.target instanceof FormControl)
          break;
        this.focusControl = null;
        break;
      case Event.Names.CLICK:
      case Event.Names.DBLCLICK:
      case Event.Names.HIDE:
      case Event.Names.KEYUP:
      case Event.Names.MOUSEDOWN:
      case Event.Names.MOUSEMOVE:
      case Event.Names.MOUSEUP:
      case Event.Names.SHOW:
        break;
      case Event.Names.FOCUS:
        if (event.target instanceof FormControl)
          break;
        // Move focus to first focusable control.
        this.focusControl = null;
        moveFocusToNext(this);
        break;
      case Event.Names.KEYDOWN:
        if (!event.defaultPrevented)
          handleKeyDown(this, /** @type {!KeyboardEvent} */(event));
        break;
      default:
        console.log('Form.handleEvent', event);
        break;
    }
  }

  Object.defineProperties(Form, {
    handleEvent: {value: handleEvent},
  });
})();
