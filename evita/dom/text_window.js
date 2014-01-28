// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @type {function() : !TextWindow} */
global.TextWindow.prototype.clone = function() {
  return new TextWindow(this.selection.range);
};

(function() {
  /** @param {!TextWindow} window */
  function selectWord(window) {
    var selection = window.selection;
    selection.startOf(Unit.WORD);
    selection.endOf(Unit.WORD, Alter.EXTEND);
    selection.startIsActive = false;
  }

  /**
   * Default event handler.
   * @this {!TextWindow}
   * @param {!Event} event.
   */
  TextWindow.handleEvent = function(event) {
    switch (event.type) {
      case Event.Names.MOUSEDOWN:
        if (/** @type {MouseEvent} */(event).ctrlKey)
          selectWord(this);
        break;
      case Event.Names.DBLCLICK:
        selectWord(this);
        break;
    }
  };
})();
