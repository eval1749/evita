// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @param {!Event} event
 * Default event handler.
 */
CheckboxControl.handleEvent = function(event) {
  var checkbox = /** @type {!CheckboxConrol} */(event.target);
  if (event.type == Event.Names.CLICK)
    checkbox.checked = !checkbox.checked;
  FormControl.handleEvent(event);
};
