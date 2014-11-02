// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /** @const @type {number} */
  const MAX_ENTRIES = 7;

  class TextFieldLogger {
    /**
     * @param {!TextFieldControl} control
     */
    constructor(control) {
      /** @private @const @type {!TextFieldControl} */
      this.control_ = control;
      /** @private @type {number} */
      this.cursor_ = 0;
      /** @private @type {!Array.<string>} */
      this.texts_ = [];
      Object.seal(TextFieldLogger);
    }

    /**
     * @param {string} value
     */
    add(value) {
      let length = this.texts_.length;
      if (length && this.texts_[length - 1] === value)
        return;
      if (length >= MAX_ENTRIES)
        this.texts_.shift();
      this.texts_.push(value);
      this.cursor_ = this.texts_.length;
    }

    resetCursor() {
      this.cursor_ = this.texts_.length;
    }

    /**
     * @param {number} direction
     */
    retrieve(direction) {
      let length = this.texts_.length;
      if (!length)
        return;
      let newCursor = this.cursor_ + direction;
      if (newCursor < 0 || newCursor >= length)
        return;
      let control = this.control_;
      if (this.cursor_ === length) {
        this.add(control.value);
        --this.cursor_;
      }
      this.cursor_ = newCursor;
      control.value = this.texts_[this.cursor_];
      control.selection.collapseTo(control.value.length);
    }
  }

  /** @type {CheckboxControl} */
  let casePreserveReplaceCheckbox = null;

  /** @type {Form} */
  let form = null;

  /** @type {FormWindow} */
  let formWindow = null;

  /** @type {CheckboxControl} */
  let matchCaseCheckbox = null;

  /** @type {CheckboxControl} */
  let matchWholeWordCheckbox = null;

  /** @type {ButtonControl} */
  let findNextButton = null;

  /** @type {ButtonControl} */
  let findPreviousButton = null;

  /** @type {TextFieldControl} */
  let findWhatText = null;

  /** @type {ButtonControl} */
  let replaceAllButton = null;

  /** @type {ButtonControl} */
  let replaceOneButton = null;

  /** @type {TextFieldControl} */
  let replaceWithText = null;

  /** @type {CheckboxControl} */
  let useRegexpCheckbox = null;

  /**
   * @param {!Window} window
   */
  function doFindNext(window) {
    let textWindow = ensureTextWindow(window);
    if (!textWindow)
      return;
    FindAndReplace.find(textWindow, findWhatText.value,
                        makeFindOptions(Direction.FORWARD));
  }

  /**
   * @param {!Window} window
   */
  function doFindPrevious(window) {
    let textWindow = ensureTextWindow(window);
    if (!textWindow)
      return;
    FindAndReplace.find(textWindow, findWhatText.value,
                        makeFindOptions(Direction.BACKWARD));
  }

  /**
   * @param {!Window} window
   */
  function doReplaceOne(window) {
    let textWindow = ensureTextWindow(window);
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
    let textWindow = ensureTextWindow(window);
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

    let BUTTON_MARGIN = 5;
    let CONTROL_HEIGHT = 26;
    let LINE_MARGIN = 5;
    let PADDING_LEFT = 5;
    let PADDING_TOP = 5;

    let controlLeft = PADDING_LEFT;
    let controlTop = PADDING_TOP;

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
      let checkbox = new CheckboxControl();
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
      let button = new ButtonControl(text);
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
      findWhatText.logger_.resetCursor();
      let canFind = findWhatText.value !== '';
      findNextButton.disabled = !canFind;
      findPreviousButton.disabled = !canFind;
      replaceOneButton.disabled = !canFind;
      replaceAllButton.disabled = !canFind;
      replaceWithText.disabled = !canFind;
    }
    findWhatText.addEventListener(Event.Names.INPUT, updateUiByFindWhat);

    function setupTextField(textField) {
      textField.addEventListener(Event.Names.CHANGE, () => {
        textField.logger_.add(textField.value);
      });
      textField.addEventListener(Event.Names.KEYDOWN, handleTextFieldKeyDown);
      textField.logger_ = new TextFieldLogger(textField);
    }

    setupTextField(findWhatText);
    setupTextField(replaceWithText);

    form.addEventListener(Event.Names.KEYDOWN, handleGlobalKeyDown);

    function installButton(button, handler) {
      button.addEventListener('click', (event) => {
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
    formWindow.realize();
  }

  /**
   * @param {!Window} windowIn
   * @return {TextWindow}
   */
  function ensureTextWindow(windowIn) {
    if (windowIn instanceof TextWindow)
      return windowIn;
    let textWindow = null;
    for (let editorWindow of EditorWindow.list) {
      for (let window of editorWindow.children) {
        if ((window instanceof TextWindow) &&
            (!textWindow || textWindow.focusTick_ < window.focusTick_)) {
          textWindow = window;
        }
      }
    }
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
    let textField = /** @type {!TextFieldControl} */(event.target);
    switch (event.keyCode) {
      case 0x126: // arrowUp
        textField.logger_.retrieve(-1);
        event.stopPropagation();
        event.preventDefault();
        break;
      case 0x128: // arrowDown
        textField.logger_.retrieve(1);
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
    for (let i = 0; i < text.length; ++i) {
      let data = Unicode.UCD[text.charCodeAt(i)];
      if (data.category === Unicode.Category.Lu ||
          data.category === Unicode.Category.Lt) {
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
      backward: direction === Direction.BACKWARD,
      casePreserveReplacement: casePreserveReplaceCheckbox.checked,
      ignoreCase: shouldIgnoreCase(findWhatText.value),
      matchWholeWord: matchWholeWordCheckbox.checked,
      useRegExp: useRegexpCheckbox.checked
   };
  }

  /**
   * @param {string} text
   * @return {boolean}
   */
  function shouldIgnoreCase(text) {
    return !hasUpperCase(text) && !matchCaseCheckbox.checked;
  }

  ////////////////////////////////////////////////////////////
  //
  // Commands
  //

  /**
   * @this {!Window}
   */
  function findNextCommand() {
    ensureForm();
    doFindNext(this);
  }

  /**
   * @this {!Window}
   */
  function findPreviousCommand() {
    ensureForm();
    doFindPrevious(this);
  }

  /**
   * @this {!Window}
   */
  function showFindFormCommand() {
    ensureForm();
    if (formWindow.state !== Window.State.REALIZED)
      return;
    formWindow.show();
    formWindow.focus();
  }

  Editor.bindKey(TextWindow, 'Ctrl+F', showFindFormCommand);
  Editor.bindKey(TextWindow, 'Ctrl+H', showFindFormCommand);
  Editor.bindKey(TextWindow, 'F3', findNextCommand);
  Editor.bindKey(TextWindow, 'Shift+F3', findPreviousCommand);
})();
