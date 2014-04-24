// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @typedef {function(!TextFieldSelection)} */
var TextFieldEditCommand;

Object.defineProperties(TextFieldControl.prototype, {
  lastChangeEventValue_: {
    value: '',
    writable: true
  },
  value: {
    /**
     * @this {!TextFieldControl}
     * @return {string}
     */
    get: function() {
      return this.value_;
    },
    /**
     * @this {!TextFieldControl}
     * @param {string} new_value
     */
    set: function(new_value) {
      if (this.value_ == new_value)
        return;
      // TODO(yosi) We should record old value for undo/redo.
      this.value_ = new_value;
      this.selection.anchorOffset = 0;
      this.selection.focusOffset = new_value.length;
    }
  },
});

(function() {
  /** @const @type {number} */
  var MAX_OFFSET = 1 << 28;

  /**
   * @constructor
   * @param {!TextFieldControl} control
   */
  function DragController(control) {
    this.control = control;
    this.dragging = false;
  }

  /** @type {!TextFieldControl} */
  DragController.prototype.control;

  /** @type {boolean} */
  DragController.prototype.dragging;

  /** @type {function()} */
  DragController.prototype.stop = function() {
    this.dragging = false;
    this.control.releaseCapture();
  };

  /** @type {function()} */
  DragController.prototype.start = function() {
    this.dragging = true;
    this.control.setCapture();
  };

  /** @type {?DragController} */
  TextFieldControl.prototype.dragController_ = null;

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

  /**
   * @param {!TextFieldControl} control
   */
  function dispatchChangeEventIfNeeded(control) {
    var value = control.value_;
    if (control.lastChangeEventValue_ == value)
      return;
    control.lastChangeEventValue_ = value;
    control.dispatchEvent(new FormEvent(Event.Names.CHANGE, {data: value}));
  }

  // TODO(yosi) Once we finish debugging of TextFieldControl editor, we should
  // not expose |bindKey|.
  TextFieldControl.bindKey = bindKey;

  /**
   * @param {!TextFieldControl} control
   * @param {!MouseEvent} event
   */
  function handleDblClick(control, event){
    if (event.button || control.form.focusControl != control)
      return;
    var offset = control.mapPointToOffset(event.clientX, event.clientY);
    if (offset < 0)
      return;
    control.selection.collapseTo(offset);
    selectWord(control);
  }

  /**
   * @param {!TextFieldControl} control
   * @param {!KeyboardEvent} event
   */
  function handleKeyDown(control, event) {
    if (event.type != Event.Names.KEYDOWN)
      return;
    var command = keymap.get(event.keyCode);
    if (!command)
      return;
    command(control.selection);
    event.preventDefault();
  }

  /**
   * @param {!TextFieldControl} control
   * @param {!MouseEvent} event
   */
  function handleMouseDown(control, event) {
    if (event.button)
      return;
    if (control.form.focusControl != control) {
      control.focus();
      return;
    }
    var offset = control.mapPointToOffset(event.clientX, event.clientY);
    if (event.shiftKey)
      control.selection.extendTo(offset);
    else
      control.selection.collapseTo(offset);
    if (event.ctrlKey)
      selectWord(control);
    if (!control.dragController_)
      control.dragController_ = new DragController(control)
    control.dragController_.start();
  }

  /**
   * @param {!TextFieldControl} control
   * @param {!MouseEvent} event
   */
  function handleMouseMove(control, event) {
    var dragController = control.dragController_;
    if (!dragController || !dragController.dragging)
      return;
    var offset = control.mapPointToOffset(event.clientX, event.clientY);
    if (offset < 0)
      return;
    control.selection.extendTo(offset);
  }

  /**
   * @param {!TextFieldControl} control
   * @param {!MouseEvent} event
   */
  function handleMouseUp(control, event) {
    if (event.button)
      return;
    stopControllers(control);
  }

  /**
   * @param {string} key_combination
   * @return {!TextFieldEditCommand}
   */
  function keyBindingOf(key_combination) {
    var key_code = Editor.parseKeyCombination(key_combination);
    return keymap.get(key_code) || function(x) {};
  }

  /**
   * @param {!TextFieldSelection} selection
   * @param {number} offset
   * @return {number}
   */
  function nextWord(selection, offset) {
    var text = selection.control.value;
    var max_offset = text.length;
    while (offset < max_offset && text.charCodeAt(offset) != Unicode.SPACE) {
      ++offset;
    }
    while (offset < max_offset && text.charCodeAt(offset) == Unicode.SPACE) {
      ++offset;
    }
    return offset;
  }

  /**
   * @param {!TextFieldSelection} selection
   * @param {number} offset
   * @return {number}
   */
  function previousWord(selection, offset) {
    var text = selection.control.value;
    while (offset && text.charCodeAt(offset - 1) == Unicode.SPACE) {
      --offset;
    }
    while (offset && text.charCodeAt(offset - 1) != Unicode.SPACE) {
      --offset;
    }
    return offset;
  }

  /**
   * @param {!TextFieldControl} control
   */
  function selectWord(control) {
    var selection = control.selection;
    var start = previousWord(selection, selection.start);
    var end = nextWord(selection, start);
    selection.collapseTo(start);
    selection.extendTo(end);
  }

  /**
   * @param {!TextFieldSelection} selection
   */
  function setSelectionText(selection, new_text) {
    var control = selection.control;
    var old_value = control.value;
    selection.text = new_text;
    var value = control.value;
    if (value == old_value)
      return;
    var event = new FormEvent(Event.Names.INPUT, {data: value});
    selection.control.dispatchEvent(event);
  }

  /**
   * @param {!TextFieldControl} control
   */
  function stopControllers(control) {
    var dragController = control.dragController_;
    if (!dragController)
      return;
    dragController.stop();
  }

  /**
   * @param {number} charCode
   * @param {!TextFieldSelection} selection
   */
  function typeCharacter(charCode, selection) {
    var control = selection.control;
    var text = control.value;
    setSelectionText(selection, String.fromCharCode(charCode));
    selection.collapseTo(selection.end);
  }

  /**
   * @this {!TextFieldControl}
   * @param {!Event} event
   * Default event handler.
   */
  TextFieldControl.handleEvent = function(event) {
    switch (event.type) {
      case Event.Names.BLUR:
        dispatchChangeEventIfNeeded(this);
        break;

      case Event.Names.DBLCLICK:
        handleDblClick(this, /** @type{!MouseEvent} */(event));
        break;
      case Event.Names.MOUSEDOWN:
        handleMouseDown(this, /** @type {!MouseEvent} */(event));
        break;
      case Event.Names.MOUSEMOVE:
        handleMouseMove(this, /** @type {!MouseEvent} */(event));
        break;
      case Event.Names.MOUSEUP:
        handleMouseUp(this, /** @type {!MouseEvent} */(event));
        break;
      case Event.Names.FOCUS:
        this.selection.collapseTo(0);
        this.selection.extendTo(this.value.length);
        break;
      case Event.Names.KEYDOWN:
        handleKeyDown(this, /** @type{!KeyboardEvent} */(event));
        break;
    }
    if (!event.defaultPrevented)
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
      selection.collapseTo(selection.end);
  });

  bindKey('Backspace', function(selection) {
    var control = selection.control;
    var text = control.value;
    if (selection.collapsed)
      --selection.focusOffset;
    setSelectionText(selection, '');
  });

  bindKey('Ctrl+A', function(selection) {
    selection.anchorOffset = 0;
    selection.focusOffset = selection.control.value.length;
  });

  bindKey('Ctrl+ArrowRight', function(selection) {
    selection.collapseTo(nextWord(selection, selection.focusOffset));
  });

  bindKey('Ctrl+ArrowLeft', function(selection) {
    selection.collapseTo(previousWord(selection, selection.focusOffset));
  });

  bindKey('Ctrl+C', function(selection) {
    if (selection.collapsed)
      return;
    var items = DataTransfer.clipboard.items;
    items.clear();
    items.add(selection.text.replace(/\n/g, '\r\n'), 'text/plain');
  });

  bindKey('Ctrl+V', function(selection) {
    var items = DataTransfer.clipboard.items;
    if (!items.length)
      return;
    var item = items.get(0);
    if (item.kind != 'string')
      return;
    setSelectionText(selection, item.getAsString());
    selection.collapseTo(selection.end);
  });

  bindKey('Ctrl+X', function(selection) {
    if (selection.collapsed)
      return;
    var items = DataTransfer.clipboard.items;
    items.clear();
    items.add(selection.text.replace(/\n/g, '\r\n'), 'text/plain');
    setSelectionText(selection, '');
  });

  bindKey('Delete', function(selection) {
    var control = selection.control;
    var text = control.value;
    if (selection.collapsed)
      ++selection.focusOffset;
    setSelectionText(selection, '');
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
    selection.extendTo(nextWord(selection, selection.focusOffset));
  });

  bindKey('Shift+Ctrl+ArrowLeft', function(selection) {
    selection.extendTo(previousWord(selection, selection.focusOffset));
  });

  bindKey('Shift+Ctrl+Delete', keyBindingOf('Ctrl+C'));
  bindKey('Shift+Delete', keyBindingOf('Ctrl+X'));
  bindKey('Shift+Insert', keyBindingOf('Ctrl+V'));

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
