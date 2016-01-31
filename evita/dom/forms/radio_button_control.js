// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @this {!RadioButtonControl}
   * @return {boolean}
   */
  function getChecked() {
    return this.checked_;
  }

  /**
   * @this {!RadioButtonControl}
   * @param {boolean} newChecked
   */
  function setChecked(newChecked) {
    if (this.checked_ === newChecked)
      return;
    this.dispatchChangeEvent();
    this.checked_ = newChecked;
    if (!this.form || !newChecked)
      return;
    // Turn checked off for other radio button in same radio button group.
    for (const other of this.form.controls) {
      if (other === this)
        continue;
      if (!(other instanceof RadioButtonControl))
        continue;
      if (other.name !== this.name)
        continue;
      if (!other.checked)
        continue;
      other.checked_ = false;
      other.dispatchChangeEvent();
      return;
    }
  }

  Object.defineProperties(RadioButtonControl.prototype, {
    checked: {get: getChecked, set: setChecked},
  });

  /**
   * @this {!RadioButtonControl}
   * @param {!Event} event
   * Default event handler.
   */
  function handleEvent(event) {
    var radioButton = /** @type {!RadioButtonControl} */(event.target);
    switch (event.type) {
      case Event.Names.CLICK:
        this.dispatchInputEvent();
        radioButton.checked = !radioButton.checked;
        radioButton.focus();
        break;
    }
    FormControl.handleEvent.call(this, event);
  }

  Object.defineProperties(RadioButtonControl, {
    handleEvent: {value: handleEvent},
  });
})();
