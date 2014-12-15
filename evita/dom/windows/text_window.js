// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @type {function() : !TextWindow} */
global.TextWindow.prototype.clone = function() {
  return new TextWindow(this.selection.range);
};

(function() {
  /** @const @type {number} */
  let AUTOSCROLL_INTERVAL_MS = 50;

  /** @const @type {number} */
  let AUTOSCROLL_MAX_MOVE = 20;

  /** @const @type {number} */
  let AUTOSCROLL_SPEED_MS = 100;

  /** @const @type {number} */
  let AUTOSCROLL_ZONE_SIZE = 20;

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
      let selection = this.window.selection;
      let current = amount > 0 ? selection.range.end : selection.range.end;
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
      this.timer.start(AUTOSCROLL_INTERVAL_MS, () => {
        let duration = Date.now() - this.startedAt;
        let amount = Math.floor(
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

  /** @type {!Autoscroller|undefined} */
  TextWindow.prototype.autoscroller_;

  /** @type {!DragController|undefined} */
  TextWindow.prototype.dragController_;

  /** @type {!Range} */
  TextWindow.prototype.textCompositionRange;

  Object.defineProperty(TextWindow.prototype, 'textCompositionRange', {
    value: null,
    writable: true
  });

  /**
   * @param {!TextWindow} window
   * @param {!CompositionEvent} event
   */
  function handleCompositionEvent(window, event) {
    if (!window.textCompositionRange)
      window.textCompositionRange = new Range(window.document);
    let range = window.textCompositionRange;
    let selection = window.selection;
    let selectionRange = selection.range;

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
      let start = span.start;
      let end = span.end;
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
    let document = window.document;
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
    Os.File.stat(document.fileName).then(function(info) {
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
              MessageBox.SETFOREGROUND).then(function(response) {
        if (response !== DialogItemId.YES) {
          document.lastWriteTime = info.lastModificationDate;
          return;
        }
        reloadDocument(window, document);
      });
    }).catch(function(reason){
      console.log('Os.File.stat', document.fileName, reason);
      document.lastStatTime_ = new Date();
      document.obsolete = Document.Obsolete.UNKNOWN;
    });
  }

  /**
   * @param {!TextWindow} window
   * @param {!UiEvent} event
   */
  function handleIdle(window, event) {
    updateStatusBar(window);
    let document = window.document;
    DocumentState.update(document);
  }

  /**
   * @param {!TextWindow} window
   * @param {!MouseEvent} event
   */
  function handleMouseDown(window, event) {
    if (event.button)
      return;
    let position = window.mapPointToPosition_(event.clientX, event.clientY);
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
    let dragController = window.dragController_;
    if (!dragController || !dragController.dragging)
      return;
    let position = window.mapPointToPosition_(event.clientX, event.clientY);
    if (position < 0)
      return;

    let selection = window.selection;
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

    let autoscroller = window.autoscroller_;
    if (!autoscroller) {
      autoscroller = new Autoscroller(window);
      window.autoscroller_ = autoscroller;
    }
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
   * @param {!WheelEvent} event
   */
  function handleWheel(window, event) {
    window.scroll(event.deltaY > 0 ? -2 : 2);
  }

  /** @param {!TextWindow} window */
  function selectWord(window) {
    let selection = window.selection;
    selection.startOf(Unit.WORD);
    selection.endOf(Unit.WORD, Alter.EXTEND);
    selection.startIsActive = false;
  }

  /**
   * @param {!TextWindow} textWindow
   * @param {!Document} document
   */
  function reloadDocument(textWindow, document) {
    let selectionMap = new Map();
    for (let window of document.listWindows()) {
      let selection = window.selection;
      selectionMap.set(selection, selection.range.start);
    }
    document.load().then(function(zero) {
      textWindow.status = 'Reloaded';
      for (let window of document.listWindows()) {
        let selection = window.selection;
        let present = selectionMap.get(selection);
        selection.range.collapseTo(present ? present : 0);
      }
    });
  }

  /** @param {!TextWindow} window */
  function stopControllers(window) {
    if (window.autoscroller_)
      window.autoscroller_.stop();
    if (window.dragController_)
      window.dragController_.stop();
  }

  /** @const @type {!Array.<string>} */
  let DOCUMENT_STATE_TEXTS = [
    'Ready', 'Loading...', 'Saving...'
  ];

  /** @const @type {!Array.<string>} */
  let NEWLINE_MODES = [
    '--', 'LF', 'CR', 'CRLF'
  ];

  /**
   * Updates status bar with TextWindow.
   * @param {!TextWindow} window
   */
  function updateStatusBar(window) {
    function equal(texts1, texts2) {
      if (texts1.length !== texts2.length)
        return false;
      // TODO(eval1749) We should use arrow notation once v8 fixes internal
      // parse error when we use arrow notation.
      return texts1.every(function(text1, index) {
        return text1 === texts2[index];
      });
    }

    let document = window.document;
    let selection = window.selection;
    let textOffset = selection.focusOffset;
    let lineAndColumn = document.getLineAndColumn_(textOffset);
    let newTexts = [
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

  /**
   * Default event handler.
   * @this {!TextWindow}
   * @param {!Event} event
   */
  TextWindow.handleEvent = function(event) {
    switch (event.type) {
      case Event.Names.BLUR:
        stopControllers(this);
        break;
      case Event.Names.DBLCLICK:
        selectWord(this);
        break;
      case Event.Names.FOCUS:
        handleFocus(this);
        break;
      case Event.Names.IDLE:
        handleIdle(this, /** @type {!UiEvent} */(event));
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
      case Event.Names.WHEEL:
        handleWheel(this, /** @type {!WheelEvent} */(event));
        break;
      default:
        if (event instanceof CompositionEvent)
          handleCompositionEvent(this, event);
        else
          Window.handleEvent.call(this, event);
        break;
    }
  };
})();
