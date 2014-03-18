// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @param {!TableWindow} window
   * @param {!MouseEvent} event
   */
  function handleDoubleClick(window, event) {
    if (event.button == 0) {
      window.document.keymap.get(KEY_CODE_MAP['enter']).call(window);
      return;
    }
  }

  /**
   * Dispalys number of selected rows in status bar.
   * @param {!TableWindow} window
   */
  function handleIdle(window) {
    // TODO(yosi) We should not count selected rows every time, we should
    // count when selection changed, e.g. handle "selectionchange" event.
    /**
     * @param {!TableSelection} selection
     * @return {number}
     */
    function countSelectedRows(selection) {
      var keys = Document.list.map(function(document) {
        return document.name;
      });
      var selected_keys = selection.getRowStates(keys).filter(
          function(state) {
            return state & TableViewRowState.SELECTED;
          });
      return selected_keys.length;
    }
    var selection = window.selection;
    var num_selected = countSelectedRows(selection);
    if (selection.lastSelectedRowCount_ == num_selected)
      return;
    selection.lastSelectedRowCount_ = num_selected;
    window.parent.setStatusBar([num_selected + ' documents']);
  }

  /**
   * Default event handler.
   * @this {!TableWindow}
   * @param {!Event} event.
   */
  TableWindow.handleEvent = function(event) {
    switch (event.type) {
      case Event.Names.DBLCLICK:
        handleDoubleClick(this, /** @type{!MouseEvent}*/(event));
        break;
      case Event.Names.IDLE:
        handleIdle(this);
        break;
    }
    Window.handleEvent(event);
  }
})();
