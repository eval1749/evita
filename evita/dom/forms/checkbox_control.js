// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @this {!CheckboxControl}
 * @param {!Event} event
 * Default event handler.
 */
CheckboxControl.handleEvent = function(event) {
  var checkbox = /** @type {!CheckboxControl} */(event.target);
  if (event.type == Event.Names.CLICK) {
    checkbox.focus();
    checkbox.checked = !checkbox.checked;
  }
  FormControl.handleEvent(event);
};
