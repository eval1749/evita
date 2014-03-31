// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @param {number} control_id
 * @return {!FormControl}
 */
global.Form.prototype.get = function(control_id) {
  var control = this.control(control_id);
  if (!control)
    throw 'No such control ' + control_id + ' in form.';
  return /** @type {!FormControl} */(control);
};

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
    form.focusControl = findFocusable(controlIndex(form, form.focusControl)) ||
                        findFocusable(-1);
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
    form.focusControl = findFocusable(controlIndex(form, form.focusControl)) ||
                        findFocusable(controls.length);
  }

  /**
   * @param {!Form} form
   * @param {!KeyboardEvent} event
   */
  function handleKeyDown(form, event) {
    if (event.ctrlKey)
      return;
    if (event.altKey) {
      var accessKey = String.fromCharCode(event.code & 0x1FF);
      form.controls.forEach(function(control) {
        if (control.accessKey == accessKey) {
          if (control instanceof ButtonControl)
            control.dispatchEvent(new MouseEvent('click'));
          else
            control.focus();
        }
      });
      return;
    }
    switch (event.code & 0x1FF) {
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
  Form.handleEvent = function(event) {
    switch (event.type) {
      case Event.Names.BLUR:
      case Event.Names.CLICK:
      case Event.Names.DBLCLICK:
      case Event.Names.IDLE:
      case Event.Names.KEYUP:
      case Event.Names.MOUSEDOWN:
      case Event.Names.MOUSEMOVE:
        break;
      case Event.Names.FOCUS:
        console.log('Form.handleEvent', this, event);
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
  };
})();
