// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('find_and_replace');

goog.scope(function() {
const FindAndReplace = find_and_replace.FindAndReplace;
const FindAndReplaceOptions = find_and_replace.FindAndReplaceOptions;
const hasUpperCase = find_and_replace.hasUpperCase;

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
    /** @const @type {number} */
    const length = this.texts_.length;
    if (length && this.texts_[length - 1] === value)
      return;
    if (length >= MAX_ENTRIES)
      this.texts_.shift();
    this.texts_.push(value);
    this.cursor_ = this.texts_.length;
  }

  resetCursor() { this.cursor_ = this.texts_.length; }

  /**
   * @param {number} direction
   */
  retrieve(direction) {
    /** @const @type {number} */
    const length = this.texts_.length;
    if (!length)
      return;
    /** @const @type {number} */
    const newCursor = this.cursor_ + direction;
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
  /** @const @type {?TextWindow} */
  const textWindow = ensureTextWindow(window);
  if (!textWindow)
    return;
  FindAndReplace.find(
      textWindow, findWhatText.value, makeFindOptions(Direction.FORWARD));
}

/**
 * @param {!Window} window
 */
function doFindPrevious(window) {
  /** @const @type {?TextWindow} */
  const textWindow = ensureTextWindow(window);
  if (!textWindow)
    return;
  FindAndReplace.find(
      textWindow, findWhatText.value, makeFindOptions(Direction.BACKWARD));
}

/**
 * @param {!Window} window
 */
function doReplaceOne(window) {
  /** @const @type {?TextWindow} */
  const textWindow = ensureTextWindow(window);
  if (!textWindow)
    return;
  FindAndReplace.replaceOne(
      textWindow, findWhatText.value, replaceWithText.value,
      makeFindOptions(Direction.FORWARD));
}

/**
 * @param {!Window} window
 */
function doReplaceAll(window) {
  /** @const @type {?TextWindow} */
  const textWindow = ensureTextWindow(window);
  if (!textWindow)
    return;
  FindAndReplace.replaceAll(
      textWindow, findWhatText.value, replaceWithText.value,
      makeFindOptions(Direction.FORWARD));
}

function ensureForm() {
  if (formWindow)
    return;

  form = new Form();

  /** @const @type {number} */
  const BUTTON_MARGIN = 5;
  /** @const @type {number} */
  const CHECKBOX_HEIGHT = 20;
  /** @const @type {number} */
  const CONTROL_HEIGHT = 26;
  /** @const @type {number} */
  const LINE_MARGIN = 5;
  /** @const @type {number} */
  const PADDING_LEFT = 5;
  /** @const @type {number} */
  const PADDING_TOP = 5;

  /** @type {number} */
  let controlLeft = PADDING_LEFT;
  /** @type {number} */
  let controlTop = PADDING_TOP;

  /**
   * @param {!FormControl} control
   * @param {number} width
   */
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
  /**
   * @param {string} text
   * @param {string} accessKey
   */
  function addCheckbox(text, accessKey) {
    /** @const @type {!CheckboxControl} */
    const checkbox = new CheckboxControl();
    checkbox.accessKey = accessKey;
    add(checkbox, CHECKBOX_HEIGHT);
    add(new LabelControl(text), text.length * 10);
    return checkbox;
  }
  matchCaseCheckbox = addCheckbox('Match case', 'C');
  newline();
  matchWholeWordCheckbox = addCheckbox('Match whole word', 'W');
  newline();
  useRegexpCheckbox = addCheckbox('Use regular expression', 'E');
  newline();
  casePreserveReplaceCheckbox = addCheckbox('Case preserve replacement', 'M');
  casePreserveReplaceCheckbox.checked = true;
  newline();

  // Buttons
  /**
   * @param {string} text
   * @param {string} accessKey
   */
  function addButton(text, accessKey) {
    /** @const @type {!ButtonControl} */
    const button = new ButtonControl(text);
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
    /** @const @type {boolean} */
    const canFind = findWhatText.value !== '';
    findNextButton.disabled = !canFind;
    findPreviousButton.disabled = !canFind;
    replaceOneButton.disabled = !canFind;
    replaceAllButton.disabled = !canFind;
    replaceWithText.disabled = !canFind;
  }
  findWhatText.addEventListener(Event.Names.INPUT, updateUiByFindWhat);

  /**
   * @param {!TextFieldControl} textField
   */
  function setupTextField(textField) {
    textField.addEventListener(Event.Names.CHANGE, function() {
      textField.logger_.add(textField.value);
    });
    textField.addEventListener(Event.Names.KEYDOWN, handleTextFieldKeyDown);
    textField.logger_ = new TextFieldLogger(textField);
  }

  setupTextField(findWhatText);
  setupTextField(replaceWithText);

  form.addEventListener(Event.Names.KEYDOWN, handleGlobalKeyDown);

  /**
   * @param {?ButtonControl} button
   * @param {!function(!Window)} handler
   */
  function installButton(button, handler) {
    if (!button)
      return;
    button.addEventListener(Event.Names.CLICK, function(event) {
      if (!Window.focus)
        return;
      handler(/**@type{!Window}*/ (Window.focus));
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
  /** @type {?TextWindow} */
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
    case 0x10D:  // Enter
      doFindNext(/** @type {!FormWindow} */ (formWindow));
      break;
    case 0x11B:  // Escape
      formWindow.hide();
      break;
  }
}

/**
 * @param {!KeyboardEvent} event
 */
function handleTextFieldKeyDown(event) {
  let textField = /** @type {!TextFieldControl} */ (event.target);
  switch (event.keyCode) {
    case 0x126:  // arrowUp
      textField.logger_.retrieve(-1);
      event.stopPropagation();
      event.preventDefault();
      break;
    case 0x128:  // arrowDown
      textField.logger_.retrieve(1);
      event.stopPropagation();
      event.preventDefault();
      break;
  }
}

/**
 * @param {!Direction} direction,
 * @return {!FindAndReplaceOptions}
 */
function makeFindOptions(direction) {
  /** @const {!FindAndReplaceOptions} */
  const options = new FindAndReplaceOptions();
  options.backward = direction === Direction.BACKWARD,
  options.casePreserveReplacement = casePreserveReplaceCheckbox.checked,
  options.ignoreCase = shouldIgnoreCase(findWhatText.value),
  options.matchWholeWord = matchWholeWordCheckbox.checked,
  options.useRegExp = useRegexpCheckbox.checked;
  return options;
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
});
