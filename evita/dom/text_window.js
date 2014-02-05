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
    if (event.clientY < window.top)
      autoscroller.start(-1);
    else if (event.clientY > window.bottom)
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

  /** @param {!TextWindow} window */
  function selectWord(window) {
    var selection = window.selection;
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
      case Event.Names.MOUSEDOWN:
        handleMouseDown(this, /** @type {!MouseEvent} */(event));
        break;
      case Event.Names.MOUSEMOVE:
        handleMouseMove(this, /** @type {!MouseEvent} */(event));
        break;
      case Event.Names.MOUSEUP:
        handleMouseUp(this, /** @type {!MouseEvent} */(event));
        break;
      default:
        Window.handleEvent(event);
        break;
    }
  };
})();
