// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @type {function() : !TextWindow} */
global.TextWindow.prototype.clone = function() {
  return new TextWindow(this.selection.range);
};

(function() {
  'use strict';

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
      const selection = this.window.selection;
      const current = amount > 0 ? selection.range.end : selection.range.end;
      selection.modify(Unit.WINDOW_LINE, amount, Alter.EXTEND);
      return amount > 0 ? current !== selection.range.end :
                          current !== selection.range.start;
    }

    stop() {
      this.timer.stop();
    }

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
        const duration = Date.now() - this.startedAt;
        const amount = Math.floor(
                        Math.min(Math.max(duration / AUTOSCROLL_SPEED_MS, 1),
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

  /* TODO(eval1749): Once closure compiler support lcoal class name, we enable
   annotation: type {Autoscroller} */
  /** @suppress {suspiciousCode} */
  TextWindow.prototype.autoscroller_;

  /* TODO(eval1749): Once closure compiler support lcoal class name, we enable
   annotation: type {DragController} */
  /** @suppress {suspiciousCode} */
  TextWindow.prototype.dragController_;

  /** @type {OneShotTimer} */
  TextWindow.prototype.selectionTimer_;

  /** @type {!Range} */
  TextWindow.prototype.textCompositionRange;

  Object.defineProperties(TextWindow.prototype, {
    'selectionTimer_': { value: null, writable: true },
    'textCompositionRange': { value: null, writable: true }
  });

  /**
   * @param {!TextWindow} window
   * @return {!Autoscroller}
   */
  function ensureAutoscroller(window) {
    const autoscroller = window.autoscroller_;
    if (autoscroller)
      return autoscroller;
    const newAutoscroller = new Autoscroller(window);
    window.autoscroller_ = newAutoscroller;
    return newAutoscroller;
  }

  /**
   * @param {!TextWindow} window
   * @param {!CompositionEvent} event
   */
  function handleCompositionEvent(window, event) {
    if (!window.textCompositionRange)
      window.textCompositionRange = new Range(window.document);
    /** @type {!Range} */
    const range = window.textCompositionRange;
    /** @type {!TextSelection} */
    const selection = /** @type {!TextSelection} */(window.selection);
    /** @type {!Range} */
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
        case 0: // ATTR_INPUT
        case 4: // ATTR_INPUT_ERROR
          selectionRange.setStyle({textDecoration: 'imeinput'});
          break;
        case 1: // ATTR_TARGET_CONVERTED
        case 3: // ATTR_TARGET_NOTCONVERTED
          selectionRange.setStyle({backgroundColor: 0x3399FF, color: 0xFFFFFF});
          break;
        case 2: // ATTR_CONVERTED
        case 5: // ATTR_FIXEDCONVERTED
          selectionRange.setStyle({textDecoration: 'imeinactive2'});
          break;
      }
    }
    selectionRange.collapseTo(range.start + event.caret);
  }

  /**
   * @param {!TextWindow} window
   */
  function handleFocus(window) {
    updateObsolete(window);
    const document = window.document;
    DocumentState.update(document);
    updateStatusBar(window);
    window.status = `Switch to ${document.name}`;
  }

  /**
   * @param {!TextWindow} window
   * @param {!MouseEvent} event
   */
  function handleMouseDown(window, event) {
    if (event.button)
      return;
    const position = window.mapPointToPosition_(event.clientX, event.clientY);
    if (position < 0)
      return;
    if (Window.focus !== window) {
      window.focus();
      if (position >= window.selection.range.start &&
          position < window.selection.range.end) {
        return
      }
    }

    if (event.shiftKey) {
      if (window.selection.startIsActive)
        window.selection.range.start = position;
      else
        window.selection.range.end = position;
    } else {
      window.selection.range.collapseTo(position);
    }

    if (event.ctrlKey) {
      selectWord(window);
      return;
    }

    if (!window.dragController_)
      window.dragController_ = new DragController(window);
    window.dragController_.start();
  }

  /**
   * @param {!TextWindow} window
   * @param {!MouseEvent} event
   */
  function handleMouseMove(window, event) {
    const dragController = window.dragController_;
    if (!dragController || !dragController.dragging)
      return;
    const position = window.mapPointToPosition_(event.clientX, event.clientY);
    if (position < 0)
      return;

    const selection = window.selection;
    if (position <= selection.range.start) {
      selection.range.start = position;
      selection.startIsActive = true;
    } else if (position >= selection.range.end) {
      selection.range.end = position;
      selection.startIsActive = false;
    } else if (selection.startIsActive) {
      selection.range.start = position;
    } else {
      selection.range.end = position;
    }

    const autoscroller = ensureAutoscroller(window);
    if (event.clientY < AUTOSCROLL_ZONE_SIZE)
      autoscroller.start(-1);
    else if (event.clientY > window.clientHeight - AUTOSCROLL_ZONE_SIZE)
      autoscroller.start(1);
    else
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
   */
  function handleSelectionChange(window) {
    if (!window.selectionTimer_)
        window.selectionTimer_ = new OneShotTimer();
    if (window.selectionTimer_.isRunning)
      return;
    // TODO(eval1749): We should use |requestIdleCallback()| rather than
    // |OneShotTimer|.
    window.selectionTimer_.start(0, () => {
      updateStatusBar(window);
      highlightMatchedBrackets(window);
    });
  }

  /**
   * @param {!TextWindow} window
   * @param {!WheelEvent} event
   */
  function handleWheel(window, event) {
    window.scroll(event.deltaY > 0 ? -2 : 2);
  }

  /**
   * @param {!Document} document
   * @param {string} key
   * @return {!Range}
   */
  function createOrGetRange(document, key) {
    const present = document.properties.get(key);
    if (present instanceof Range)
      return present;
    const newRange = new Range(document);
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
    function isLeftBracket(range) {
      if (range.start == range.document.length)
        return false;
      const leftBracket = Bracket.DATA[document.charCodeAt_(range.start)];
      return leftBracket && leftBracket.type === Bracket.Type.LEFT;
    }

    function isRightBracket(range) {
      if (range.start === 0)
        return false;

      const rightBracket = Bracket.DATA[document.charCodeAt_(range.start - 1)];
      return rightBracket && rightBracket.type === Bracket.Type.RIGHT;
    }

    function tryLeft(range) {
      range.moveEnd(Unit.BRACKET, 1);
      return !range.collapsed;
    }

    function tryRight(range) {
      range.moveStart(Unit.BRACKET, -1);
      return !range.collapsed;
    }

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

    const selection = /** @type {!TextSelection} */(window.selection);
    const selectionRange = selection.range;
    const document = selection.document;
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
   * @param {!TextWindow} textWindow
   * @param {!Document} document
   */
  function reloadDocument(textWindow, document) {
    const selectionMap = new Map();
    for (let window of document.listWindows()) {
      const selection = window.selection;
      selectionMap.set(selection, selection.range.start);
    }
    document.load().then((zero) => {
      textWindow.status = 'Reloaded';
      for (let window of document.listWindows()) {
        const selection = window.selection;
        const present = selectionMap.get(selection);
        selection.range.collapseTo(present ? present : 0);
      }
    });
  }

  /** @param {!TextWindow} window */
  function selectWord(window) {
    const selection = window.selection;
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
  const DOCUMENT_STATE_TEXTS = [
    'Ready', 'Loading...', 'Saving...'
  ];

  /** @const @type {!Array.<string>} */
  const NEWLINE_MODES = [
    '--', 'LF', 'CR', 'CRLF'
  ];
  function updateObsolete(window) {
    const document = window.document;
    if (document.fileName === '')
      return;
    if (document.obsolete === Document.Obsolete.CHECKING) {
      window.status = 'Checking file status...';
      return;
    }
    if (document.obsolete === Document.Obsolete.YES) {
      Editor.messageBox(window, 'This document has been stale.',
                        MessageBox.ICONWARNING);
      return;
    }
    document.obsolete = Document.Obsolete.CHECKING;
    Os.File.stat(document.fileName).then((info) => {
      document.lastStatTime_ = new Date();
      document.obsolete = info.lastModificationDate.valueOf() ===
                            document.lastWriteTime.valueOf() ?
          Document.Obsolete.NO : Document.Obsolete.YES;
      if (document.obsolete === Document.Obsolete.NO) {
        window.status = 'Healthy';
        return;
      }
      Editor.messageBox(window, 'This document is stale.',
                        MessageBox.ICONWARNING);
      Editor.messageBox(window,
          Editor.localizeText(Strings.IDS_ASK_REFRESH, {name: document.name}),
          MessageBox.YESNO | MessageBox.ICONWARNING | MessageBox.TOPMOST |
              MessageBox.SETFOREGROUND).then((response) => {
        if (response !== DialogItemId.YES) {
          document.lastWriteTime = info.lastModificationDate;
          return;
        }
        reloadDocument(window, document);
      });
    }).catch((reason) => {
      console.log('Os.File.stat', document.fileName, reason);
      document.lastStatTime_ = new Date();
      document.obsolete = Document.Obsolete.UNKNOWN;
    });
  }

  /**
   * Updates status bar with TextWindow.
   * @param {!TextWindow} window
   */
  function updateStatusBar(window) {
    function equal(texts1, texts2) {
      if (texts1.length !== texts2.length)
        return false;
      // TODO(eval1749): We should use arrow notation once v8 fixes internal
      // parse error when we use arrow notation.
      return texts1.every((text1, index) => {
        return text1 === texts2[index];
      });
    }

    const document = window.document;
    const selection = window.selection;
    const textOffset = selection.focusOffset;
    const lineAndColumn = document.getLineAndColumn_(textOffset);
    const newTexts = [
      DOCUMENT_STATE_TEXTS[document.state],
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
  };

  /** @type {!Map.<string, !function(!TextWindow, !Event)>} */
  const handlerMap = new Map([
    [ Event.Names.BLUR, stopControllers ],
    [ Event.Names.DBLCLICK, selectWord ],
    [ Event.Names.FOCUS, handleFocus ],
    [ Event.Names.MOUSEDOWN, handleMouseDown ],
    [ Event.Names.MOUSEMOVE, handleMouseMove ],
    [ Event.Names.MOUSEUP, handleMouseUp ],
    [ Event.Names.SELECTIONCHANGE, handleSelectionChange ],
    [ Event.Names.WHEEL, handleWheel ]
  ]);

  /**
   * Default event handler.
   * @this {!TextWindow}
   * @param {!Event} event
   */
  TextWindow.handleEvent = function (event) {
    let handler = handlerMap.get(event.type);
    if (handler)
      handler(this, event);
    if (event instanceof CompositionEvent) {
      handleCompositionEvent(this, event);
      return
    }
    Window.handleEvent.call(this, event);
  };
})();
