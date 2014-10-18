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
  var casePreserveReplaceCheckbox;

  /** @type {?Form} */
  var form = null;

  /** @type {?FormWindow} */
  var formWindow = null;

  /** @type {?CheckboxControl} */
  var matchCaseCheckbox;

  /** @type {?CheckboxControl} */
  var matchWholeWordCheckbox;

  /** @type {?ButtonControl} */
  var findNextButton;

  /** @type {?ButtonControl} */
  var findPreviousButton;

  /** @type {?TextFieldControl} */
  var findWhatText;

  /** @type {?ButtonControl} */
  var replaceAllButton;

  /** @type {?ButtonControl} */
  var replaceOneButton;

  /** @type {?TextFieldControl} */
  var replaceWithText;

  /** @type {?CheckboxControl} */
  var useRegexpCheckbox;

  /**
   * @param {!Window} window
   */
  function doFindNext(window) {
    var textWindow = ensureTextWindow(window);
    if (!textWindow)
      return;
    FindAndReplace.find(textWindow, findWhatText.value,
                        makeFindOptions(Direction.FORWARD));
  }

  /**
   * @param {!Window} window
   */
  function doFindPrevious(window) {
    var textWindow = ensureTextWindow(window);
    if (!textWindow)
      return;
    FindAndReplace.find(textWindow, findWhatText.value,
                        makeFindOptions(Direction.BACKWARD));
  }

  /**
   * @param {!Window} window
   */
  function doReplaceOne(window) {
    var textWindow = ensureTextWindow(window);
    if (!textWindow)
      return;
    FindAndReplace.replaceOne(textWindow, findWhatText.value,
                              replaceWithText.value,
                              makeFindOptions(Direction.FORWARD));
  }

  /**
   * @param {!Window} window
   */
  function doReplaceAll(window) {
    var textWindow = ensureTextWindow(window);
    if (!textWindow)
      return;
    FindAndReplace.replaceAll(textWindow, findWhatText.value,
                              replaceWithText.value,
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
    findWhatText = add(new TextFieldControl(), 265);
    findWhatText.accessKey = 'N';
    newline();
    add(new LabelControl('Replace with:'), 75);
    replaceWithText = add(new TextFieldControl(), 265);
    replaceWithText.accessKey = 'P';
    replaceWithText.disabled = true;
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
    matchCaseCheckbox = addCheckbox('Match case', 'C');
    newline();
    matchWholeWordCheckbox = addCheckbox('Match whole word', 'W');
    newline();
    useRegexpCheckbox = addCheckbox('Use regular expression', 'E');
    newline();
    casePreserveReplaceCheckbox = addCheckbox('Case preserve replacement',
                                                 'M');
    casePreserveReplaceCheckbox.checked = true;
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
    replaceOneButton = addButton('Replace', 'R');
    findPreviousButton = addButton('Find Previous', 'I');
    newline();
    controlLeft = 140;
    replaceAllButton = addButton('Replace All', 'A');
    findNextButton = addButton('Find Next', 'F');

    function updateUiByFindWhat() {
      findWhatText.logger.resetCursor();
      var canFind = findWhatText.value != '';
      findNextButton.disabled = !canFind;
      findPreviousButton.disabled = !canFind;
      replaceOneButton.disabled = !canFind;
      replaceAllButton.disabled = !canFind;
      replaceWithText.disabled = !canFind;
    }
    findWhatText.addEventListener(Event.Names.INPUT, updateUiByFindWhat);

    function setupTextField(textField) {
      textField.addEventListener(Event.Names.CHANGE, function() {
        textField.logger.add(textField.value);
      });
      textField.addEventListener(Event.Names.KEYDOWN, handleTextFieldKeyDown);
      textField.logger = new TextFieldLogger(textField);
    }

    setupTextField(findWhatText);
    setupTextField(replaceWithText);

    form.addEventListener(Event.Names.KEYDOWN, handleGlobalKeyDown);

    function installButton(button, handler) {
      button.addEventListener('click', function(event) {
        if (!Window.focus)
          return;
        handler(/**@type{!Window}*/(Window.focus));
      });
    }

    installButton(findNextButton, doFindNext);
    installButton(findPreviousButton, doFindPrevious);
    installButton(replaceOneButton, doReplaceOne);
    installButton(replaceAllButton, doReplaceAll);

    updateUiByFindWhat();
    formWindow = new FormWindow(form);

    // Set focus on |findWhatText| control when find dialog box is activated.
    formWindow.addEventListener(Event.Names.FOCUS, function() {
      findWhatText.focus();
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
      casePreserveReplacement: casePreserveReplaceCheckbox.checked,
      ignoreCase: shouldIgnoreCase(findWhatText.value),
      matchWholeWord: matchWholeWordCheckbox.checked,
      useRegExp: useRegexpCheckbox.checked
   };
  }

  /**
   * @this {!Window}
   */
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
    return !hasUpperCase(text) && !matchCaseCheckbox.checked;
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
