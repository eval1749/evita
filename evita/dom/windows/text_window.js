// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

/** @const @type {number} */
const AUTOSCROLL_INTERVAL_MS = 50;

/** @const @type {number} */
const AUTOSCROLL_MAX_MOVE = 20;

/** @const @type {number} */
const AUTOSCROLL_SPEED_MS = 100;

/** @const @type {number} */
const AUTOSCROLL_ZONE_SIZE = 20;

//////////////////////////////////////////////////////////////////////
//
// Autoscroller
//
class Autoscroller {
  /**
   * @param {!TextWindow} window
   */
  constructor(window) {
    /** @type {number} */
    this.direction = 0;
    /** @type {number} */
    this.startedAt = Date.now();
    /** @type {!Timer} */
    this.timer = new RepeatingTimer();
    /** @type {!TextWindow} */
    this.window = window;
  }

  /**
   * @param {number} amount
   */
  scroll(amount) {
    /** @const @type {!TextSelection} */
    const selection = this.window.selection;
    /** @const @type {!number} */
    const current = amount > 0 ? selection.range.end : selection.range.end;
    selection.modify(Unit.WINDOW_LINE, amount, Alter.EXTEND);
    return amount > 0 ? current !== selection.range.end :
                        current !== selection.range.start;
  }

  stop() { this.timer.stop(); }

  /**
   * @param {number} direction
   */
  start(direction) {
    if (this.timer.isRunning) {
      if (this.direction !== direction) {
        this.direction = direction;
        this.startedAt = Date.now();
      }
      return;
    }
    this.direction = direction;
    this.startedAt = Date.now();
    this.timer.start(AUTOSCROLL_INTERVAL_MS, function() {
      /** @const @type {number} */
      const duration = Date.now() - this.startedAt;
      /** @const @type {number} */
      const amount = Math.floor(
          Math.min(
              Math.max(duration / AUTOSCROLL_SPEED_MS, 1),
              AUTOSCROLL_MAX_MOVE));
      if (!this.scroll(amount * this.direction))
        this.stop();
    }, this);
  }
}

//////////////////////////////////////////////////////////////////////
//
// DragController
//
class DragController {
  /** * @param {!TextWindow} window */
  constructor(window) {
    /** @type {boolean} */
    this.dragging = false;
    /** @type {!TextWindow} */
    this.window = window;
  }

  stop() {
    this.dragging = false;
    this.window.releaseCapture();
  }

  start() {
    this.dragging = true;
    this.window.setCapture();
  }
}

/** @type {?Autoscroller} */
TextWindow.prototype.autoscroller_;

/** @type {?DragController} */
TextWindow.prototype.dragController_;

/** @type {boolean} */
TextWindow.prototype.isSelectionChanged_;

/** @type {!TextRange} */
TextWindow.prototype.textCompositionRange;

/**
 * @this {!TextWindow}
 */
function cloneTextWindow() {
  return new TextWindow(this.selection.range);
}

/**
 * @param {!TextWindow} window
 * @return {!Autoscroller}
 */
function ensureAutoscroller(window) {
  /** @const @type {?Autoscroller} */
  const autoscroller = window.autoscroller_;
  if (autoscroller)
    return autoscroller;
  /** @const @type {!Autoscroller} */
  const newAutoscroller = new Autoscroller(window);
  window.autoscroller_ = newAutoscroller;
  return newAutoscroller;
}

/**
 * @param {!TextWindow} window
 * @return {!DragController}
 */
function ensureDragController(window) {
  /** @const @type {?DragController} */
  const dragController = window.dragController_;
  if (dragController)
    return dragController;
  /** @const @type {!DragController} */
  const newDragController = new DragController(window);
  window.dragController_ = newDragController;
  return newDragController;
}

/**
 * @param {!TextWindow} window
 * @param {!CompositionEvent} event
 */
function handleCompositionEvent(window, event) {
  if (!window.textCompositionRange)
    window.textCompositionRange = new TextRange(window.document);
  /** @type {!TextRange} */
  const range = window.textCompositionRange;
  /** @type {!TextSelection} */
  const selection = /** @type {!TextSelection} */ (window.selection);
  /** @type {!TextRange} */
  const selectionRange = selection.range;

  if (event.type === Event.Names.COMPOSITIONSTART) {
    range.collapseTo(selectionRange.start);
    return;
  }

  if (event.type === Event.Names.COMPOSITIONCOMMIT) {
    range.text = event.data;
    range.collapseTo(range.end);
    selectionRange.collapseTo(range.start);
    return;
  }

  if (event.type !== Event.Names.COMPOSITIONUPDATE)
    return;
  range.text = event.data;
  // Decorate composition text.
  for (let span of event.spans) {
    const start = span.start;
    const end = span.end;
    if (start === end)
      continue;
    selectionRange.collapseTo(range.start + start);
    selectionRange.end = range.start + end;
    switch (span.data) {
      case 0:  // ATTR_INPUT
      case 4:  // ATTR_INPUT_ERROR
        selectionRange.setStyle({textDecoration: 'imeinput'});
        break;
      case 1:  // ATTR_TARGET_CONVERTED
      case 3:  // ATTR_TARGET_NOTCONVERTED
        selectionRange.setStyle({backgroundColor: 0x3399FF, color: 0xFFFFFF});
        break;
      case 2:  // ATTR_CONVERTED
      case 5:  // ATTR_FIXEDCONVERTED
        selectionRange.setStyle({textDecoration: 'imeinactive2'});
        break;
    }
  }
  selectionRange.collapseTo(range.start + event.caret);
}

/**
 * @param {!TextWindow} window
 * @param {!Event} event
 */
function handleDestroy(window, event) {
  /** @const @type {!TextDocumentEvent} */
  const detachEvent = new TextDocumentEvent(Event.Names.DETACH, {view: window});
  window.document.dispatchEvent(detachEvent);
}

/**
 * @param {!TextWindow} window
 * @param {!MouseEvent} event
 *
 * Selects word at left button double clicking.
 */
function handleDoubleClick(window, event) {
  if (event.button)
    return;
  /** @const @type {number} */
  const offset = mapPointToOffset(event);
  if (offset < 0)
    return;
  selectWordAt(window, offset);
}

/**
 * @param {!TextWindow} window
 */
function handleFocus(window) {
  Editor.requestIdleCallback(updateObsolete.bind(window), {timeout: 500});
  updateStatusBar(window, `Switch to ${window.document.name}`);
}

/**
 * @param {!TextWindow} window
 * @param {!MouseEvent} event
 */
function handleMouseDown(window, event) {
  if (event.button)
    return;
  if (Window.focus !== window)
    window.focus();
  /** @const @type {number} */
  const offset = mapPointToOffset(event);
  if (offset < 0)
    return;
  if (event.shiftKey) {
    if (window.selection.startIsActive)
      window.selection.range.start = offset;
    else
      window.selection.range.end = offset;
    return;
  }
  if (event.ctrlKey) {
    selectWordAt(window, offset);
    return;
  }
  window.selection.range.collapseTo(offset);
  ensureDragController(window).start();
}

/**
 * @param {!TextWindow} window
 * @param {!MouseEvent} event
 */
function handleMouseMove(window, event) {
  /** @const @type {number} */
  const offset = mapPointToOffset(event);
  /** @const @type {?DragController} */
  const dragController = window.dragController_;
  if (offset < 0 || !dragController || !dragController.dragging)
    return;

  /** @const @type {!TextSelection} */
  const selection = window.selection;
  if (offset <= selection.range.start) {
    selection.range.start = offset;
    selection.startIsActive = true;
  } else if (offset >= selection.range.end) {
    selection.range.end = offset;
    selection.startIsActive = false;
  } else if (selection.startIsActive) {
    selection.range.start = offset;
  } else {
    selection.range.end = offset;
  }

  const autoscroller = ensureAutoscroller(window);
  if (event.clientY < AUTOSCROLL_ZONE_SIZE) {
    autoscroller.start(-1);
    return;
  }
  if (event.clientY > window.clientHeight - AUTOSCROLL_ZONE_SIZE) {
    autoscroller.start(1);
    return;
  }
  autoscroller.stop();
}

/**
 * @param {!TextWindow} window
 * @param {!MouseEvent} event
 */
function handleMouseUp(window, event) {
  if (event.button)
    return;
  stopControllers(window);
}

/**
 * @param {!TextWindow} window
 * @param {!Event} event
 */
function handleRealize(window, event) {
  /** @const @type {!TextDocumentEvent} */
  const attachEvent = new TextDocumentEvent(Event.Names.ATTACH, {view: window});
  window.document.dispatchEvent(attachEvent);
}

/**
 * @param {!TextWindow} window
 */
function handleSelectionChange(window) {
  if (window.isSelectionChanged_)
    return;
  window.isSelectionChanged_ = true;
  Editor.requestIdleCallback(() => {
    window.isSelectionChanged_ = false;
    updateStatusBar(window, 'Ready');
    highlightMatchedBrackets(window);
  }, {timeout: 100});
}

/**
 * @param {!TextWindow} window
 * @param {!WheelEvent} event
 */
function handleWheel(window, event) {
  // TODO(eval1749): We should retrieve number of scroll line from
  // |SPI_GETWHEELSCROLLLINES|.
  /** @type {number} */ const lines = 3;
  window.scroll(event.deltaY > 0 ? -lines : lines);
}

/**
 * @param {!TextDocument} document
 * @param {string} key
 * @return {!TextRange}
 */
function createOrGetRange(document, key) {
  const present = document.properties.get(key);
  if (present instanceof TextRange)
    return present;
  /** @param {!TextRange} range */
  const newRange = new TextRange(document);
  document.properties.set(key, newRange);
  return newRange;
}

/** @type {number}*/
global.matchedBracketsColor = 0xEEFF41;

/**
 * Highlight matched brackets at selection.
 * @param {!TextWindow} window
 */
function highlightMatchedBrackets(window) {
  /** @param {!TextRange} range */
  function isLeftBracket(range) {
    if (range.start == range.document.length)
      return false;
    const leftBracket = Bracket.DATA[document.charCodeAt(range.start)];
    return leftBracket && leftBracket.type === Bracket.Type.LEFT;
  }

  /** @param {!TextRange} range */
  function isRightBracket(range) {
    if (range.start === 0)
      return false;
    const rightBracket = Bracket.DATA[document.charCodeAt(range.start - 1)];
    return rightBracket && rightBracket.type === Bracket.Type.RIGHT;
  }

  /** @param {!TextRange} range */
  function tryLeft(range) {
    range.moveEnd(Unit.BRACKET, 1);
    return !range.collapsed;
  }

  /** @param {!TextRange} range */
  function tryRight(range) {
    range.moveStart(Unit.BRACKET, -1);
    return !range.collapsed;
  }

  /** @param {!TextRange} range */
  function colorMatching(range) {
    const rangeStart = range.start;
    const rangeEnd = range.end;

    range.collapseTo(rangeStart + 1);
    range.start = rangeStart;
    range.setStyle({backgroundColor: global.matchedBracketsColor});

    range.collapseTo(rangeEnd);
    range.start = rangeEnd - 1;
    range.setStyle({backgroundColor: global.matchedBracketsColor});

    range.start = rangeStart;
  }

  function reportNoMatching(window) {
    window.status = Strings.IDS_NO_MATCHING_PAREN;
  }

  /** @const @type {!TextSelection} */
  const selection = /** @type {!TextSelection} */ (window.selection);
  /** @const @type {!TextRange} */
  const selectionRange = selection.range;
  /** @const @type {!TextDocument} */
  const document = selection.document;
  /** @const @type {!TextRange} */
  const range = createOrGetRange(document, 'bracket');
  // TODO(eval1749): We should just remove 'backgroundColor' style from
  // range.
  range.setStyle({backgroundColor: 0xFFFFFF});
  if (!selectionRange.collapsed)
    return;

  range.collapseTo(selectionRange.start);
  if (isLeftBracket(range)) {
    if (tryLeft(range))
      return colorMatching(range);
    return reportNoMatching(window);
  }

  if (!isRightBracket(range))
    return;
  if (tryRight(range))
    return colorMatching(range);
  return reportNoMatching(window);
}

/**
 * @param {!MouseEvent} event
 * @return {number}
 */
function mapPointToOffset(event) {
  /** @const @type {!TextWindow} */
  const textWindow = /** @type {!TextWindow} */ (event.target);
  return textWindow.hitTestPoint(event.clientX, event.clientY);
}

/**
 * @param {!TextWindow} textWindow
 * @param {!TextDocument} document
 */
function reloadTextDocument(textWindow, document) {
  /** @const @type {!Map<!TextSelection, number>} */
  const selectionMap = new Map();
  for (/** @type {!Window} */ let window of document.listWindows()) {
    if (!(window instanceof TextWindow))
      continue;
    /** @const @type {!TextWindow} */
    const textWindow = /** @const @type {!TextWindow} */ (window);
    /** @const @type {!TextSelection} */
    const selection = textWindow.selection;
    selectionMap.set(selection, selection.range.start);
  }
  document.load().then((zero) => {
    textWindow.status = 'Reloaded';
    for (let window of document.listWindows()) {
      /** @const @type {!TextSelection} */
      const selection = window.selection;
      /** @const @type {number} */
      const present = selectionMap.get(selection) || 0;
      selection.range.collapseTo(present ? present : 0);
    }
  });
}

/**
 * @param {!TextWindow} window
 * @param {number} offset
 */
function selectWordAt(window, offset) {
  /** @const @type {!TextSelection} */
  const selection = window.selection;
  selection.range.collapseTo(offset);
  selection.startOf(Unit.WORD);
  selection.endOf(Unit.WORD, Alter.EXTEND);
  selection.startIsActive = false;
}

/** @param {!TextWindow} window */
function stopControllers(window) {
  if (window.autoscroller_)
    window.autoscroller_.stop();
  if (window.dragController_)
    window.dragController_.stop();
}

/** @const @type {!Array.<string>} */
const DOCUMENT_STATE_TEXTS = ['Ready', 'Loading...', 'Saving...'];

/** @const @type {!Array.<string>} */
const NEWLINE_MODES = ['--', 'LF', 'CR', 'CRLF'];

/**
 * @this {!TextWindow}
 */
function updateObsolete() {
  /** @const @type {!TextWindow} */
  const window = this;
  /** @const @type {!TextDocument} */
  const document = window.document;
  if (document.fileName === '')
    return;
  if (document.obsolete === TextDocument.Obsolete.CHECKING) {
    window.status = 'Checking file status...';
    return;
  }
  if (document.obsolete === TextDocument.Obsolete.YES) {
    Editor.messageBox(
        window, 'This document has been stale.', MessageBox.ICONWARNING);
    return;
  }
  document.obsolete = TextDocument.Obsolete.CHECKING;
  Os.File.stat(document.fileName)
      .then((info) => {
        document.lastStatTime_ = new Date();
        document.obsolete = info.lastModificationDate.valueOf() ===
                document.lastWriteTime.valueOf() ?
            TextDocument.Obsolete.NO :
            TextDocument.Obsolete.YES;
        if (document.obsolete === TextDocument.Obsolete.NO)
          return;
        Editor.messageBox(
            window, 'This document is stale.', MessageBox.ICONWARNING);
        Editor
            .messageBox(
                window, Editor.localizeText(
                            Strings.IDS_ASK_REFRESH, {name: document.name}),
                MessageBox.YESNO | MessageBox.ICONWARNING | MessageBox.TOPMOST |
                    MessageBox.SETFOREGROUND)
            .then((response) => {
              if (response !== DialogItemId.YES) {
                document.lastWriteTime = info.lastModificationDate;
                return;
              }
              reloadTextDocument(window, document);
            });
      })
      .catch((reason) => {
        console.log('Os.File.stat', document.fileName, reason);
        document.lastStatTime_ = new Date();
        document.obsolete = TextDocument.Obsolete.UNKNOWN;
      });
}

/**
 * Updates status bar with TextWindow.
 * @param {!TextWindow} window
 * @param {string} stateText
 */
function updateStatusBar(window, stateText) {
  function equal(texts1, texts2) {
    if (texts1.length !== texts2.length)
      return false;
    return texts1.every((text1, index) => text1 === texts2[index]);
  }

  /** @const @type {!TextDocument} */
  const document = window.document;
  /** @const @type {!TextSelection} */
  const selection = window.selection;
  /** @const @type {number} */
  const textOffset = selection.focusOffset;
  /** @const @type {!LineAndColumn} */
  const lineAndColumn = document.getLineAndColumn_(textOffset);
  /** @const @type {!Array<string>} */
  const newTexts = [
    document.state === 0 ? stateText : DOCUMENT_STATE_TEXTS[document.state],
    document.mode ? document.mode.name : '--',
    document.encoding ? document.encoding : 'n/a',
    NEWLINE_MODES[document.newline],
    'Ln ' + lineAndColumn.lineNumber,
    'Col ' + lineAndColumn.column,
    'Ch ' + textOffset,
    document.readOnly ? 'R/O' : 'INS',
  ];

  // We call |setStatusBar()| only if status bar contents are changed.
  if (window.statusBarTexts_ && equal(window.statusBarTexts_, newTexts))
    return;
  window.statusBarTexts_ = newTexts;
  window.parent.setStatusBar(newTexts);
}

/** @type {!Map.<string, !function(!TextWindow, !Event)>} */
const handlerMap = new Map([
  [Event.Names.BLUR, stopControllers],
  [Event.Names.DBLCLICK, handleDoubleClick],
  [Event.Names.DESTROY, handleDestroy], [Event.Names.FOCUS, handleFocus],
  [Event.Names.MOUSEDOWN, handleMouseDown],
  [Event.Names.MOUSEMOVE, handleMouseMove],
  [Event.Names.MOUSEUP, handleMouseUp], [Event.Names.REALIZE, handleRealize],
  [Event.Names.SELECTIONCHANGE, handleSelectionChange],
  [Event.Names.WHEEL, handleWheel]
]);

/**
 * Default event handler.
 * @this {!TextWindow}
 * @param {!Event} event
 */
function handleEvent(event) {
  /** @const @type {?function(!TextWindow, !Event)} */
  const handler = handlerMap.get(event.type) || null;
  if (handler)
    handler(this, event);
  if (event instanceof CompositionEvent) {
    handleCompositionEvent(this, event);
    return
  }
  Window.handleEvent.call(this, event);
};

Object.defineProperties(TextWindow.prototype, {
  autoscroller_: {value: null, writable: true},
  dragController_: {value: null, writable: true},
  isSelectionChanged_: {value: false, writable: true},
  selectionTimer_: {value: null, writable: true},
  textCompositionRange: {value: null, writable: true},

  // Additional methods
  clone: {value: cloneTextWindow},
});

Object.defineProperties(TextWindow, {
  handleEvent: {value: handleEvent},
});
});
