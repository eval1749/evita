// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @this {!CheckboxControl}
   * @return {boolean}
   */
  function getChecked() {
    return this.checked_;
  }

  /**
   * @this {!CheckboxControl}
   * @param {boolean} newChecked
   */
  function setChecked(newChecked) {
    if (this.checked_ === newChecked)
      return;
    this.dispatchChangeEvent();
    this.checked_ = newChecked;
  }

  Object.defineProperties(CheckboxControl.prototype, {
    checked: {get: getChecked, set: setChecked},
  });

  /**
   * @this {!CheckboxControl}
   * @param {!Event} event
   * Default event handler.
   */
  function handleEvent(event) {
    var checkbox = /** @type {!CheckboxControl} */(event.target);
    switch (event.type) {
      case Event.Names.CLICK:
        this.dispatchInputEvent();
        checkbox.checked = !checkbox.checked;
        checkbox.focus();
        break;
    }
    FormControl.handleEvent.call(this, event);
  }

  Object.defineProperties(CheckboxControl, {
    handleEvent: {value: handleEvent},
  });
})();
