// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @param {number} control_id
 * @return {!FormControl}
 */
Form.prototype.get = function(control_id) {
  var control = this.control(control_id);
  if (!control)
    throw 'No such control ' + control_id + ' in form.';
  return /** @type {!FormControl} */(control);
};
