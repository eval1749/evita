// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @return {boolean}
 */
global.FormControl.prototype.canFocus = function() {
  return !this.disabled;
};

/**
 * @param {!Event} event
 * Default event handler.
 */
FormControl.handleEvent = function(event) {
  // Redirect event to |Form.handleEvent| if control is associated to form.
  var control = /** @type {!FormControl} */(event.target);
  var form = control.form;
  if (form) {
    Form.handleEvent.call(form, event);
    return;
  }
  switch (event.type) {
    case Event.Names.IDLE:
    case Event.Names.MOUSEMOVE:
      break;
    default:
      console.log('FormControl.handleEvent', event);
      break;
  }
};
