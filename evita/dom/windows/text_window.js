// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @type {function() : !TextWindow} */
global.TextWindow.prototype.clone = function() {
  return new TextWindow(this.selection.range);
};

(function() {
  /** @const @type {number} */
  var AUTOSCROLL_INTERVAL_MS = 50;

  /** @const @type {number} */
  var AUTOSCROLL_SPEED_MS = 100;

  /** @const @type {number} */
  var AUTOSCROLL_MAX_MOVE = 20;

  /**
   * @constructor
   * @param {!TextWindow} window
   */
  function Autoscroller(window) {
    this.direction = 0;
    this.startedAt = Date.now();
    this.timer = new RepeatingTimer();
    this.window = window;
  }

  /** @type {number} */
  Autoscroller.prototype.duration;

  /** @type {number} */
  Autoscroller.prototype.startedAt;

  /** @type {Timer} */
  Autoscroller.prototype.timer;

  /** @type {!TextWindow} */
  Autoscroller.prototype.window;

  /**
   * @param {number} amount
   */
  Autoscroller.prototype.scroll = function(amount) {
    var selection = this.window.selection;
    var current = amount > 0 ? selection.range.end : selection.range.end;
    selection.modify(Unit.WINDOW_LINE, amount, Alter.EXTEND);
    return amount > 0 ? current != selection.range.end :
                        current != selection.range.start;
  };

  /**
   * @type {function()}
   */
  Autoscroller.prototype.stop = function() {
    this.timer.stop();
  };

  /**
   * @param {number} direction
   */
  Autoscroller.prototype.start = function(direction) {
    if (this.timer.isRunning) {
      if (this.direction != direction) {
        this.direction = direction;
        this.startedAt = Date.now();
      }
      return;
    }
    this.direction = direction;
    this.startedAt = Date.now();
    this.timer.start(AUTOSCROLL_INTERVAL_MS, function() {
      var duration = Date.now() - this.startedAt;
      var amount = Math.floor(
                      Math.min(Math.max(duration / AUTOSCROLL_SPEED_MS, 1),
                               AUTOSCROLL_MAX_MOVE));
      if (!this.scroll(amount * this.direction))
        this.stop();
    }, this);
  };

  /**
   * @constructor
   * @param {!TextWindow} window
   */
  function DragController(window) {
    this.dragging = false;
    this.window = window;
  };

  /** @type {boolean} */
  DragController.prototype.dragging;

  /** @type {!TextWindow} */
  DragController.prototype.window;

  /** @type{function(number)} */
  DragController.prototype.onMouseMove = function(event) {
    if (!this.dragging)
      return;
    this.window.setCapture();
  };

  /** @type {function()} */
  DragController.prototype.stop = function() {
    this.dragging = false;
    this.window.releaseCapture();
  };

  /** @type {function()} */
  DragController.prototype.start = function() {
    this.dragging = true;
    this.window.setCapture();
  };

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
    var range = window.textCompositionRange;
    var selection = window.selection;
    var selectionRange = selection.range;

    if (event.type == Event.Names.COMPOSITIONSTART) {
      range.collapseTo(selectionRange.start);
      return;
    }

    if (event.type == Event.Names.COMPOSITIONCOMMIT) {
      range.text = event.data;
      range.collapseTo(range.end);
      selectionRange.collapseTo(range.start);
      return;
    }

    if (event.type != Event.Names.COMPOSITIONUPDATE)
      return;
    range.text = event.data;
    var last_attr = 0;
    var attrs = event.attributes;
    var start = 0;
    var selectionStart = -1;
    var selectionEnd = -1;
    function setStyle(start, end, attr) {
      if (start == end)
        return;
      selectionRange.collapseTo(range.start+ start);
      selectionRange.end = range.start + end;
      // ATTR_INPUT=0, ATTR_TARGET_CONVERTED=1, ATTR_CONVERTED=2,
      // ATTR_TARGET_NOTCONVERTED=3, ATTR_INPUT_ERROR=4
      // ATTR_FIXEDCONVERTED=5
      switch (attr) {
        case 0: // ATTR_INPUT
          selectionRange.setStyle({textDecoration: 'imeinput'});
          break;
        case 1: // ATTR_TARGET_CONVERTED
        case 3: // ATTR_TARGET_NOTCONVERTED
          if (selectionStart < 0)
            selectionStart = start;
          selectionEnd = end;
          selectionRange.setStyle({backgroundColor: 0x3399FF, color: 0xFFFFFF});
          break;
        case 2: // ATTR_CONVERTED
          selectionRange.setStyle({textDecoration: 'imeinactive2'});
          break;
      }
    }
    for (var index = 0; index < attrs.length; ++index) {
      var attr = attrs[index];
      if (!index || last_attr != attr) {
        setStyle(start, index, last_attr);
        start = index;
        last_attr = attr;
      }
    }
    setStyle(start, attrs.length, last_attr);
    selectionRange.collapseTo(range.start + event.caret);
  }

  /**
   * @param {!TextWindow} window
   */
  function handleFocus(window) {
    stopControllers(window);
    var document = window.document;
    if (document.fileName == '')
      return;
    if (document.obsolete == Document.Obsolete.CHECKING) {
      window.status = 'Checking file status...';
      return;
    }
    if (document.obsolete == Document.Obsolete.YES) {
      Editor.messageBox(window, 'This document has been stale.',
                        MessageBox.ICONWARNING);
      return;
    }
    document.obsolete = Document.Obsolete.CHECKING;
    Os.File.stat(document.fileName).then(function(info) {
      document.lastStatTime_ = new Date();
      document.obsolete = info.lastModificationDate.valueOf() ==
                            document.lastWriteTime.valueOf() ?
          Document.Obsolete.NO : Document.Obsolete.YES;
      if (document.obsolete == Document.Obsolete.NO) {
        window.status = 'Healthy';
        return;
      }
      Editor.messageBox(window, 'This document is stale.',
                        MessageBox.ICONWARNING);
      Editor.messageBox(window,
          Editor.localizeText(Strings.IDS_ASK_REFRESH, {name: document.name}),
          MessageBox.YESNO | MessageBox.ICONWARNING | MessageBox.TOPMOST |
              MessageBox.SETFOREGROUND).then(function(response) {
        if (response != DialogItemId.YES) {
          document.lastWriteTime = info.lastModificationDate;
          return;
        }
        reloadDocument(window, document);
      });
    }).catch(function(reason) {
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
    var lastIdleTimeStamp = window.lastIdleTimeStamp_;
    if (lastIdleTimeStamp) {
      var duration_sec = event.timeStamp - lastIdleTimeStamp;
      var duration_ms = Math.floor(duration_sec * 1000);
      if (duration_ms > 1000) {
        // We may want to know some process take long time.
        window.status = 'Idle ' + event.detail + ' ' + duration_ms + 'ms';
      }
    }
    window.lastIdleTimeStamp_ = event.timeStamp;
    updateStatusBar(window);
    var document = window.document;
    DocumentState.update(document);
    document.doColor_(300);
  }

  /**
   * @param {!TextWindow} window
   * @param {!MouseEvent} event
   */
  function handleMouseDown(window, event) {
    if (event.button)
      return;
    var position = window.mapPointToPosition_(event.clientX, event.clientY);
    if (position < 0)
      return;
    if (Window.focus != window) {
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
      selectWord(this);
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
    var dragController = window.dragController_;
    if (!dragController || !dragController.dragging)
      return;
    var position = window.mapPointToPosition_(event.clientX, event.clientY);
    if (position < 0)
      return;

    var selection = window.selection;
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

    var autoscroller = window.autoscroller_;
    if (!autoscroller) {
      autoscroller = new Autoscroller(window);
      window.autoscroller_ = autoscroller;
    }
    if (event.clientY < window.clientTop)
      autoscroller.start(-1);
    else if (event.clientY > window.clientTop + window.clientHeight)
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
    var selection = window.selection;
    selection.startOf(Unit.WORD);
    selection.endOf(Unit.WORD, Alter.EXTEND);
    selection.startIsActive = false;
  }

  /**
   * @param {!TextWindow} window
   * @param {!Document} document
   */
  function reloadDocument(window, document) {
    var selection_map = new Map();
    document.listWindows().forEach(function(window) {
      var selection = window.selection;
      selection_map.set(selection, selection.range.start);
    })
    document.load().then(function(zero) {
      window.status = 'Reloaded';
      document.listWindows().forEach(function (window) {
        var selection = window.selection;
        var present = selection_map.get(selection);
        selection.range.collapseTo(present ? present : 0);
      });
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
  var DOCUMENT_STATE_TEXTS = [
    'Ready', 'Loading...', 'Saving...'
  ];

  /** @const @type {!Array.<string>} */
  var NEWLINE_MODES = [
    '--', 'LF', 'CR', 'CRLF'
  ];

  /**
   * Updates status bar with TextWindow.
   * @param {!TextWindow} window
   */
  function updateStatusBar(window) {
    var document = window.document;
    var selection = window.selection;
    var text_offset = selection.focusOffset;
    var line_and_column = document.getLineAndColumn_(text_offset);
    var new_texts = [
      DOCUMENT_STATE_TEXTS[document.state],
      document.mode ? document.mode.name : '--',
      document.encoding ? document.encoding : 'n/a',
      NEWLINE_MODES[document.newline],
      'Ln ' + line_and_column.lineNumber,
      'Col ' + line_and_column.column,
      'Ch ' + text_offset,
      document.readOnly ? 'R/O' : 'INS',
    ];

    // We call |setStatusBar()| only if status bar contents are changed.
    if (window.statusBarTexts_ == new_texts)
      return;
    window.statusBarTexts_ = new_texts;
    window.parent.setStatusBar(new_texts);
  };

  /**
   * Default event handler.
   * @this {!TextWindow}
   * @param {!Event} event.
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
          Window.handleEvent(event);
        break;
    }
  };
})();
