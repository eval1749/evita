// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @param {!Event} event
 * Default event handler.
 */
FormControl.handleEvent = function(event) {
    switch (event.type) {
      case Event.Names.IDLE:
      case Event.Names.MOUSEMOVE:
        break;
      default:
        console.log('FormControl.handleEvent', event);
        break;
    }
};
