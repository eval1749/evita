// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!DocumentState=} opt_state
 */
global.TabData = function (opt_state) {
  if (!arguments.length) {
    this.icon = 0;
    this.state = 0;
    this.title = '';
    this.tooltip = '';
    return;
  }
  var state = /** @type{!DocumentState} */(opt_state);
  var tooltip_texts = [
    'Name: ' + state.name,
    'File: ' + (state.filename ? state.filename : 'no file'),
    'Save: ' + (state.lastWriteTime.valueOf() ?
        state.lastWriteTime.toLocaleString() : 'never saved'),
    state.modified ? (state.filename ? 'Not saved' : 'Modified') :
        'Not modified',
  ];
  this.icon = state.icon;
  this.state = state.state;
  this.title = state.name;
  this.tooltip =  tooltip_texts.join('\n');
};

(function() {
  /**
   * @type {!Map.<!Window, !DocumentState>}
   */
  var tabStateMap = new Map();

  /**
   * For testing purpose only.
   * @param {!Window} window
   * @return {?TabData}
   */
  global.TabData.get = function(window) {
    return tabStateMap.get(window);
  };

  /**
   * This function is called when document is attached to window.
   *
   * @param {!Window} window
   * @param {!DocumentState} state
   */
  global.TabData.update = function(window, state) {
    if (!(window instanceof DocumentWindow))
      return;
    updateTabData(window, state);
  };

  /**
   * @param {!DocumentWindow} window
   */
  function updateTabData(window, state) {
    var tab_state = tabStateMap.get(window);
    if (tab_state && tab_state.equals(state))
      return;
    Editor.setTabData(window, new TabData(state));
    tabStateMap.set(window, state);
  }

  /**
   * Updates tab data for windows attached to |document| with new |state| when
   * |DocumentState| is changed.
   *
   * @param {!Document} document
   * @param {!DocumentState} state
   */
  DocumentState.addObserver(function(document, state) {
    document.listWindows().forEach(function(window) {
      updateTabData(window, state);
    });
  });
})();
