// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @param {!TableWindow} window
   * @param {!MouseEvent} event
   */
  function handleDoubleClick(window, event) {
    if (event.button === 0) {
      const enter_key = /** @type{number} */(KEY_CODE_MAP.get('enter'));
      window.document.keymap.get(enter_key).call(window);
      return;
    }
  }

  /**
   * Displays number of selected rows in status bar.
   * @param {!TableWindow} window
   */
  function handleSelectionChange(window) {
    /**
     * @param {!TableSelection} selection
     * @return {number}
     */
    function countSelectedRows(selection) {
      var keys = TextDocument.list.map(function(document) {
        return document.name;
      });
      var selected_keys = selection.getRowStates(keys).filter(
          function(state) {
            return state & TableViewRowState.SELECTED;
          });
      return selected_keys.length;
    }
    var selection = /** @type{!TableSelection}*/(window.selection);
    var selectedCount = countSelectedRows(selection);
    if (selection.lastSelectedRowCount_ == selectedCount)
      return;
    selection.lastSelectedRowCount_ = selectedCount;
    window.parent.setStatusBar([`${selectedCount} documents`]);
  }

  /**
   * Default event handler.
   * @this {!TableWindow}
   * @param {!Event} event
   */
  TableWindow.handleEvent = function(event) {
    switch (event.type) {
      case Event.Names.DBLCLICK:
        handleDoubleClick(this, /** @type{!MouseEvent}*/(event));
        break;
      case Event.Names.FOCUS:
      case Event.Names.SELECTIONCHANGE:
        handleSelectionChange(this);
        break;
    }
    Window.handleEvent.call(this, event);
  }
})();
