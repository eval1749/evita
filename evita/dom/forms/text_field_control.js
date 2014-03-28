// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @param {!TextFieldControl} control
   * @param {!KeyboardEvent} event
   */
  function handleKeyboardEvent(control, event) {
    var selection = control.selection;
    switch (event.code & 0x1FF) {
      case 0x125: // Left
        if (event.ctrlKey)
          --selection.focusOffset;
        else
          selection.collapseTo(selection.focusOffset - 1);
        event.preventDefault();
        break;
      case 0x127: // Right
        if (event.ctrlKey)
          ++selection.focusOffset;
        else
          selection.collapseTo(selection.focusOffset + 1);
        event.preventDefault();
        break;
      default:
        if (event.code >= 0x20 && event.code <= 0x7E) {
          event.target.value += String.fromCharCode(event.code);
          event.preventDefault();
        }
        break;
    }
  }

  /**
   * @param {!Event} event
   * Default event handler.
   */
  TextFieldControl.handleEvent = function(event) {
    if (event instanceof KeyboardEvent)
      handleKeyboardEvent(this, event);
    FormControl.handleEvent(event);
  };
})();
