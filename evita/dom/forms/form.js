// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @param {!Form} form
   * @param {?FormControl} control
   * @return {number}
   */
  function controlIndex(form, control) {
    if (!control)
      return -1;
    var controls = form.controls;
    for (var i = 0; i < controls.length; ++i) {
      if (controls[i] == control)
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
    var controls = form.controls;
    function findFocusable(start) {
      for (var i = start + 1; i < controls.length; ++i) {
        var control = controls[i];
        if (control.canFocus())
          return control;
      }
      return null;
    }
    var focusControl = findFocusable(controlIndex(form, form.focusControl)) ||
                       findFocusable(-1);
    if (focusControl)
      focusControl.focus();
  }

  /**
   * Move focus to previous focusable control in |form|. If no control has
   * focus, we set focus to first focusable control. Order of control is as
   * same as index of control in |Form.prototype.controls| vector.
   * @param {!Form} form
   */
  function moveFocusToPrevious(form) {
    var controls = form.controls;
    function findFocusable(start) {
      for (var i = start - 1; i >= 0; --i) {
        var control = controls[i];
        if (control.canFocus())
          return control;
      }
      return null;
    }
    var focusControl = findFocusable(controlIndex(form, form.focusControl)) ||
                       findFocusable(controls.length);
    if (focusControl)
      focusControl.focus();
  }

  /**
   * @param {!Form} form
   * @param {!KeyboardEvent} event
   */
  function handleKeyDown(form, event) {
    if (event.ctrlKey)
      return;
    if (event.altKey) {
      // Set focus to control which have matched access key for controls other
      // than button, checkbox and radio button:
      //  * button: dispatch "click" event.
      //  * checkbox: toggle |checked| property.
      //  * radio button: toggle |checked| property.
      var accessKey = String.fromCharCode(event.keyCode & 0x1FF);
      var control = form.controls.find(function(control) {
        return control.accessKey == accessKey;
      });
      if (!control || control.disabled)
        return;
      if (control instanceof ButtonControl) {
        control.dispatchEvent(new MouseEvent('click'));
        return;
      }
      control.focus();
      if (control instanceof CheckboxControl)
        control.checked = !control.checked;
      else if (control instanceof RadioButtonControl)
        control.checked = !control.checked;
      return;
    }
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
        if (!this.focusControl)
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
