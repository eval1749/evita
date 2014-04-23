// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @param {!Event} event
 * Default event handler for RadioButtonControl.
 */
global.RadioButtonControl.handleEvent = function(event) {
  var radio_button = /** @type {!RadioButtonControl} */(event.target);
  switch (event.type) {
    case Event.Names.CLICK:
      radio_button.checked = !radio_button.checked;
      radio_button.focus();
      break;
  }
  FormControl.handleEvent(event);
};
