// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


(function() {
  /**
   * Default event handler.
   * @this {!FormWindow}
   * @param {!Event} event
   */
  FormWindow.handleEvent = function(event) {
    Form.handleEvent.call(this.form, event);
    Window.handleEvent(event);
  };
})();
