// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @typedef {function(!TextFieldSelection)} */
var TextFieldEditCommand;

/** @type {number} */
TextFieldControl.prototype.compositionStart_;

/** @type {string} */
TextFieldControl.prototype.compositionString_;

Object.defineProperties(TextFieldControl.prototype, {
  compositionStart_: {value: 0, writable: true},
  compositionString_: {value: '', writable: true},
  lastChangeEventValue_: {value: '', writable: true},
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
  const MAX_OFFSET = 1 << 28;

  class DragController {
    /**
     * @constructor
     * @param {!TextFieldControl} control
     */
    constructor(control) {
      this.control = control;
      this.dragging = false;
    }

    stop() {
      this.dragging = false;
      this.control.releaseCapture();
    }

    start() {
      this.dragging = true;
      this.control.setCapture();
    }
  }

  /** @type {!TextFieldControl} */
  DragController.prototype.control;

  /** @type {boolean} */
  DragController.prototype.dragging;

  /* TODO(eval1749) Once closure compiler support lcoal class name, we enable
   annotation: type {DragController} */
  TextFieldControl.prototype.dragController_ = null;

  /** @type {!Map.<number, !TextFieldEditCommand>} */
  const keymap = new Map();

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
   * @param {!CompositionEvent} event
   */
  function handleCompositionUpdate(control, event) {
    setCompoistionText(control, event.data);
    control.selection.collapseTo(control.compositionStart_ + event.caret);
  }

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
   * @param {!TextFieldControl} control
   * @param {string} newText
   */
  function setCompoistionText(control, newText) {
    var start = control.compositionStart_;
    var maxOffset = control.value_.length;
    if (start > maxOffset) {
      control.compositionStart_ = maxOffset;
      start = maxOffset;
    }
    var end = start + control.compositionString_.length;
    setText(control, start, end, newText);
  }

  /**
   * @param {!TextFieldSelection} selection
   * @param {string} newText
   */
  function setSelectionText(selection, newText) {
    var start = selection.start;
    setText(selection.control, selection.start, selection.end, newText);
    selection.collapseTo(start + newText.length);
  }

  /**
   * @param {!TextFieldControl} control
   * @param {string} newText
   */
  function setText(control, start, end, newText) {
    var text = control.value_;
    var compositionEnd = control.compositionStart_ +
                         control.compositionString_.length;
    var oldValue = control.value_;
    var newValue = text.substr(0, start) + newText + text.substr(end);
    control.compositionString_ = newText;
    control.value_ = newValue;
    var event = new FormEvent(Event.Names.INPUT, {data: newValue});
    control.dispatchEvent(event);
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
      case Event.Names.COMPOSITIONCOMMIT:
        setCompoistionText(this, /** @type{!CompositionEvent} */(event).data);
        this.selection.collapseTo(this.compositionStart_ +
                                  this.compositionString_.length);
        return;
      case Event.Names.COMPOSITIONEND:
        return;
      case Event.Names.COMPOSITIONSTART:
        this.selection.text = '';
        this.compositionStart_ = this.selection.anchorOffset;
        this.compositionString_ = '';
        return;
      case Event.Names.COMPOSITIONUPDATE:
        handleCompositionUpdate(this, /** @type{!CompositionEvent} */(event));
        return;
      case Event.Names.DBLCLICK:
        handleDblClick(this, /** @type{!MouseEvent} */(event));
        break;
      case Event.Names.FOCUS:
        this.selection.collapseTo(0);
        this.selection.extendTo(this.value.length);
        break;
      case Event.Names.KEYDOWN:
        handleKeyDown(this, /** @type{!KeyboardEvent} */(event));
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

  bindKey('Ctrl+Backspace', function(selection) {
    if (selection.collapsed)
      selection.extendTo(previousWord(selection, selection.focusOffset));
    setSelectionText(selection, '');
  });

  bindKey('Ctrl+C', function(selection) {
    if (selection.collapsed)
      return;
    var items = DataTransfer.clipboard.items;
    items.clear();
    items.add(selection.text.replace(/\n/g, '\r\n'), 'text/plain');
  });

  bindKey('Ctrl+Delete', function(selection) {
    if (selection.collapsed)
      selection.extendTo(nextWord(selection, selection.focusOffset));
    setSelectionText(selection, '');
  });

  bindKey('Ctrl+Shift+ArrowRight', function(selection) {
    selection.extendTo(nextWord(selection, selection.focusOffset));
  });

  bindKey('Ctrl+Shift+ArrowLeft', function(selection) {
    selection.extendTo(previousWord(selection, selection.focusOffset));
  });

  bindKey('Ctrl+Shift+Delete', keyBindingOf('Ctrl+C'));

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
