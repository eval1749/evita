// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @param {!TableWindow} window
   * @param {!MouseEvent} event
   */
  function handleDoubleClick(window, event) {
    if (event.button == 0) {
      window.document.keymap.get(KEY_CODE_MAP['enter']).call(window);
      return;
    }
  }

  /**
   * Default event handler.
   * @this {!TableWindow}
   * @param {!Event} event.
   */
  TableWindow.handleEvent = function(event) {
    if (event.type == 'dblclick') {
      handleDoubleClick(this, /** @type{!MouseEvent}*/(event));
      return;
    }
    Window.handleEvent(event);
  }
})();
