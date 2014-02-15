// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

Form.prototype.get = function(control_id) {
  var present = null;
  this.controls.forEach(function(control) {
    if (control.controlId == control_id)
      present = control;
  });
  if (!present)
    throw 'No such control ' + control_id + ' in form.';
  return /** @type {!FormControl} */(present);
};
