// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @typedef {function(!TextFieldSelection)} */
var TextFieldEditCommand;

(function() {
  /** @const @type {number} */
  var MAX_OFFSET = 1 << 28;

  /** @type {!Map.<number, !TextFieldEditCommand>} */
  var keymap = new Map();


  /**
   * @param {string} key_combination
   * @param {!TextFieldEditCommand} command
   */
  function bindKey(key_combination, command) {
    var key_code = Editor.parseKeyCombination(key_combination);
    keymap.set(key_code, command);
  }

  // TODO(yosi) Once we finish debugging of TextFieldControl editor, we should
  // not expose |bindKey|.
  TextFieldControl.bindKey = bindKey;

  /**
   * @param {!TextFieldSelection} selection
   * @return {number}
   */
  function nextWord(selection) {
    var text = selection.control.value;
    var max_offset = text.length;
    var offset = selection.focusOffset;
    while (offset < max_offset && text.charCodeAt(offset) != 0x20) {
      ++offset;
    }
    while (offset < max_offset && text.charCodeAt(offset) == 0x20) {
      ++offset;
    }
    return offset;
  }

  /**
   * @param {!TextFieldSelection} selection
   * @return {number}
   */
  function previousWord(selection) {
    var text = selection.control.value;
    var offset = selection.focusOffset - 1;
    while (offset && text.charCodeAt(offset - 1) == 0x20) {
      --offset;
    }
    while (offset && text.charCodeAt(offset - 1) != 0x20) {
      --offset;
    }
    return offset;
  }

  /**
   * @param {number} charCode
   * @param {!TextFieldSelection} selection
   */
  function typeCharacter(charCode, selection) {
    var control = selection.control;
    var text = control.value;
    if (selection.collapsed) {
      control.value = text.substr(0, selection.focusOffset) +
                      String.fromCharCode(charCode) +
                      text.substr(selection.focusOffset);
      selection.collapseTo(selection.focusOffset + 1);
      return;
    }
    control.value = text.substr(0, selection.start) +
                    String.fromCharCode(charCode) +
                    text.substr(selection.end);
    selection.collapseTo(selection.start + 1);
  }

  /**
   * @param {!TextFieldControl} control
   * @param {!KeyboardEvent} event
   */
  function handleKeyboardEvent(control, event) {
    var command = keymap.get(event.code);
    if (!command)
      return;
    command(control.selection);
    event.preventDefault();
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

  //////////////////////////////////////////////////////////////////////
  //
  // Key bindings
  //
  bindKey('ArrowLeft', function(selection) {
    if (selection.collapsed)
      selection.collapseTo(selection.focusOffset - 1);
    else
      selection.collapseTo(selection.start);
  });

  bindKey('ArrowRight', function(selection) {
    if (selection.collapsed)
      selection.collapseTo(selection.focusOffset + 1);
    else
      selection.collapseTo(selection.start);
  });

  bindKey('Backspace', function(selection) {
    var control = selection.control;
    var text = control.value;
    if (selection.collapsed) {
      var offset = selection.focusOffset;
      if (!offset)
        return;
      control.value = text.substr(0, offset - 1) + text.substr(offset);
      selection.collapseTo(offset - 1);
      return;
    }
    control.value = text.substr(0, selection.start) +
                    text.substr(selection.end);
  });

  bindKey('Ctrl+A', function(selection) {
    selection.anchorOffset = 0;
    selection.focusOffset = selection.control.value.length;
  });

  bindKey('Ctrl+ArrowRight', function(selection) {
    selection.collapseTo(nextWord(selection));
  });

  bindKey('Ctrl+ArrowLeft', function(selection) {
    selection.collapseTo(previousWord(selection));
  });

  bindKey('Delete', function(selection) {
    var control = selection.control;
    var text = control.value;
    if (selection.collapsed) {
      control.value = text.substr(0, selection.focusOffset) +
                      text.substr(selection.focusOffset + 1);
      return;
    }
    control.value = text.substr(0, selection.start) +
                    text.substr(selection.end);
  });

  bindKey('End', function(selection) {
    selection.collapseTo(MAX_OFFSET);
  });

  bindKey('Home', function(selection) {
    selection.collapseTo(0);
  });

  bindKey('Shift+ArrowLeft', function(selection) {
    --selection.focusOffset;
  });

  bindKey('Shift+ArrowRight', function(selection) {
    ++selection.focusOffset;
  });

  bindKey('Shift+Ctrl+ArrowRight', function(selection) {
    selection.extendTo(nextWord(selection));
  });

  bindKey('Shift+Ctrl+ArrowLeft', function(selection) {
    selection.extendTo(previousWord(selection));
  });

  bindKey('Shift+End', function(selection) {
    selection.focusOffset = MAX_OFFSET;
  });

  bindKey('Shift+Home', function(selection) {
    selection.focusOffset = 0;
  });

  for (var charCode = 0x20; charCode <= 0x7E; ++charCode) {
    keymap.set(charCode, typeCharacter.bind(undefined, charCode));
  }
})();
