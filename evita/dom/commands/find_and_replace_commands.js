// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @constructor
   * @param {!TextFieldControl} control
   */
  function TextFieldLogger(control) {
    this.control = control;
    this.cursor = 0;
    this.texts = [];
  }

  /** @const @type {number} */
  TextFieldLogger.MAX_ENTRIES = 7;

  /**
   * @param {string} value
   */
  TextFieldLogger.prototype.add = function(value) {
    var length = this.texts.length;
    if (length && this.texts[length - 1] == value)
      return;
    if (length >= TextFieldLogger.MAX_ENTRIES)
      this.texts.shift();
    this.texts.push(value);
    this.cursor = this.texts.length;
  };

  TextFieldLogger.prototype.resetCursor = function() {
    this.cursor = this.texts.length;
  };

  /**
   * @param {number} direction
   */
  TextFieldLogger.prototype.retrieve = function(direction) {
    var length = this.texts.length;
    if (!length)
      return;
    var newCursor = this.cursor + direction;
    if (newCursor < 0 || newCursor >= length)
      return;
    var control = this.control;
    if (this.cursor == length) {
      this.add(control.value);
      --this.cursor;
    }
    this.cursor = newCursor;
    control.value = this.texts[this.cursor];
    control.selection.collapseTo(control.value.length);
  };

  /** @type {?CheckboxControl} */
  var casePreserve_replaceCheckbox;

  /** @type {?Form} */
  var form = null;

  /** @type {?FormWindow} */
  var formWindow = null;

  /** @type {?CheckboxControl} */
  var matchCase_checkbox;

  /** @type {?CheckboxControl} */
  var matchWhole_wordCheckbox;

  /** @type {?ButtonControl} */
  var findNext_button;

  /** @type {?ButtonControl} */
  var findPrevious_button;

  /** @type {?TextFieldControl} */
  var findWhat_text;

  /** @type {?ButtonControl} */
  var replaceAll_button;

  /** @type {?ButtonControl} */
  var replaceOne_button;

  /** @type {?TextFieldControl} */
  var replaceWith_text;

  /** @type {?CheckboxControl} */
  var useRegexp_checkbox;

  /**
   * @param {!Window} window
   */
  function doFindNext(window) {
    var textWindow = ensureTextWindow(window);
    if (!textWindow)
      return;
    FindAndReplace.find(textWindow, findWhat_text.value,
                        makeFindOptions(Direction.FORWARD));
  }

  /**
   * @param {!Window} window
   */
  function doFindPrevious(window) {
    var textWindow = ensureTextWindow(window);
    if (!textWindow)
      return;
    FindAndReplace.find(textWindow, findWhat_text.value,
                        makeFindOptions(Direction.BACKWARD));
  }

  /**
   * @param {!Window} window
   */
  function doReplaceOne(window) {
    var textWindow = ensureTextWindow(window);
    if (!textWindow)
      return;
    FindAndReplace.replaceOne(textWindow, findWhat_text.value,
                              replaceWith_text.value,
                              makeFindOptions(Direction.FORWARD));
  }

  /**
   * @param {!Window} window
   */
  function doReplaceAll(window) {
    var textWindow = ensureTextWindow(window);
    if (!textWindow)
      return;
    FindAndReplace.replaceAll(textWindow, findWhat_text.value,
                              replaceWith_text.value,
                              makeFindOptions(Direction.FORWARD));
  }

  function ensureForm() {
    if (formWindow)
      return;

    form = new Form();

    var BUTTON_MARGIN = 5;
    var CONTROL_HEIGHT = 26;
    var LINE_MARGIN = 5;
    var PADDING_LEFT = 5;
    var PADDING_TOP = 5;

    var controlLeft = PADDING_LEFT;
    var controlTop = PADDING_TOP;

    function add(control, width) {
      control.clientLeft = controlLeft;
      control.clientTop = controlTop;
      control.clientWidth = width;
      control.clientHeight = CONTROL_HEIGHT;
      controlLeft += control.clientWidth;
      form.add(control);
      return control;
    }

    function newline() {
      controlLeft = PADDING_LEFT;
      controlTop += CONTROL_HEIGHT + LINE_MARGIN;
    }

    form.title = 'Find and Replace - evita';
    form.width = 350;
    form.height = 235;

    // Text fields
    add(new LabelControl('Find what:'), 75);
    findWhat_text = add(new TextFieldControl(), 265);
    findWhat_text.accessKey = 'N';
    newline();
    add(new LabelControl('Replace with:'), 75);
    replaceWith_text = add(new TextFieldControl(), 265);
    replaceWith_text.accessKey = 'P';
    replaceWith_text.disabled = true;
    newline();

    // Checkboxes
    function addCheckbox(text, accessKey) {
      var checkbox = new CheckboxControl();
      checkbox.accessKey = accessKey;
      add(checkbox, 20);
      add(new LabelControl(text), text.length * 10);
      return checkbox;
    }
    CONTROL_HEIGHT = 20;
    matchCase_checkbox = addCheckbox('Match case', 'C');
    newline();
    matchWhole_wordCheckbox = addCheckbox('Match whole word', 'W');
    newline();
    useRegexp_checkbox = addCheckbox('Use regular expression', 'E');
    newline();
    casePreserve_replaceCheckbox = addCheckbox('Case preserve replacement',
                                                 'M');
    casePreserve_replaceCheckbox.checked = true;
    newline();

    // Buttons
    CONTROL_HEIGHT = 26;
    function addButton(text, accessKey) {
      var button = new ButtonControl(text);
      button.accessKey = accessKey;
      add(button, 100);
      controlLeft += BUTTON_MARGIN;
      return button;
    }
    controlLeft = 140;
    replaceOne_button = addButton('Replace', 'R');
    findPrevious_button = addButton('Find Previous', 'I');
    newline();
    controlLeft = 140;
    replaceAll_button = addButton('Replace All', 'A');
    findNext_button = addButton('Find Next', 'F');

    function updateUiByFindWhat() {
      findWhat_text.logger.resetCursor();
      var canFind = findWhat_text.value != '';
      findNext_button.disabled = !canFind;
      findPrevious_button.disabled = !canFind;
      replaceOne_button.disabled = !canFind;
      replaceAll_button.disabled = !canFind;
      replaceWith_text.disabled = !canFind;
    }
    findWhat_text.addEventListener(Event.Names.INPUT, updateUiByFindWhat);

    function setupTextField(textField) {
      textField.addEventListener(Event.Names.CHANGE, function() {
        textField.logger.add(textField.value);
      });
      textField.addEventListener(Event.Names.KEYDOWN, handleTextFieldKeyDown);
      textField.logger = new TextFieldLogger(textField);
    }

    setupTextField(findWhat_text);
    setupTextField(replaceWith_text);

    form.addEventListener(Event.Names.KEYDOWN, handleGlobalKeyDown);

    function installButton(button, handler) {
      button.addEventListener('click', function(event) {
        if (!Window.focus)
          return;
        handler(/**@type{!Window}*/(Window.focus));
      });
    }

    installButton(findNext_button, doFindNext);
    installButton(findPrevious_button, doFindPrevious);
    installButton(replaceOne_button, doReplaceOne);
    installButton(replaceAll_button, doReplaceAll);

    updateUiByFindWhat();
    formWindow = new FormWindow(form);

    // Set focus on |findWhat_text| control when find dialog box is activated.
    formWindow.addEventListener(Event.Names.FOCUS, function() {
      findWhat_text.focus();
    });
    formWindow.realize();
    // TODO(yosi) |global.findForm| is only for debugging purpose.
    global.findFormWindow = formWindow;
  }

  /**
   * @param {!Window} window
   * @return {?TextWindow}
   */
  function ensureTextWindow(window) {
    if (window instanceof TextWindow)
      return window;
    var textWindow = null;
    EditorWindow.list.forEach(function(editorWindow) {
      editorWindow.children.forEach(function(window) {
        if (!(window instanceof TextWindow))
          return;
        if (!textWindow || textWindow.focusTick_ < window.focusTick_)
          textWindow = window;
      });
    });
    return textWindow;
  }

  /**
   * @param {!KeyboardEvent} event
   */
  function handleGlobalKeyDown(event) {
    switch (event.keyCode) {
      case 0x10D: // Enter
        doFindNext(/** @type {!FormWindow} */(formWindow));
        break;
      case 0x11B: // Escape
        formWindow.hide();
        break;
    }
  }

  /**
   * @param {!KeyboardEvent} event
   */
  function handleTextFieldKeyDown(event) {
    var textField = /** @type {!TextFieldControl} */(event.target);
    switch (event.keyCode) {
      case 0x126: // arrowUp
        textField.logger.retrieve(-1);
        event.stopPropagation();
        event.preventDefault();
        break;
      case 0x128: // arrowDown
        textField.logger.retrieve(1);
        event.stopPropagation();
        event.preventDefault();
        break;
    }
  }

  /**
   * @param {string} text
   * @return {boolean}
   */
  function hasUpperCase(text) {
    for (var i = 0; i < text.length; ++i) {
      var data = Unicode.UCD[text.charCodeAt(i)];
      if (data.category == Unicode.Category.Lu ||
          data.category == Unicode.Category.Lt) {
        return true;
      }
    }
    return false;
  }

  /**
   * @param {!Direction} direction,
   * @return {!FindAndReplaceOptions}
   */
  function makeFindOptions(direction) {
    return {
      backward: direction == Direction.BACKWARD,
      casePreserveReplacement: casePreserve_replaceCheckbox.checked,
      ignoreCase: shouldIgnoreCase(findWhat_text.value),
      matchWholeWord: matchWhole_wordCheckbox.checked,
      useRegExp: useRegexp_checkbox.checked
   };
  }

  function showFindFormCommand() {
    ensureForm();
    if (formWindow.state != Window.State.REALIZED)
      return;
    formWindow.show();
    formWindow.focus();
  }

  /**
   * @param {string} text
   * @return {boolean}
   */
  function shouldIgnoreCase(text) {
    return !hasUpperCase(text) && !matchCase_checkbox.checked;
  }

  Editor.bindKey(TextWindow, 'Ctrl+F', showFindFormCommand);
  Editor.bindKey(TextWindow, 'Ctrl+H', showFindFormCommand);

  Editor.bindKey(TextWindow, 'F3', function() {
    ensureForm();
    doFindNext(this);
  });

  Editor.bindKey(TextWindow, 'Shift+F3', function() {
    ensureForm();
    doFindPrevious(this);
  });
})();
