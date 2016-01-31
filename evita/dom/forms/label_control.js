// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @return {boolean}
 */
global.LabelControl.prototype.canFocus = function() {
  return false;
};

/**
 * @this {!LabelControl}
 * @param {!Event} event
 * Default event handler.
 */
global.LabelControl.handleEvent = function(event) {
  FormControl.handleEvent.call(this, event);
};
