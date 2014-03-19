// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!DocumentState} doc_state
 */
global.TabData = function (doc_state) {
  var tooltip_texts = [
    'Name: ' + doc_state.name,
    'File: ' + (doc_state.filename ? doc_state.filename : 'no file'),
    'Save: ' + (doc_state.lastWriteTime.valueOf() ?
        doc_state.lastWriteTime.toLocaleString() : 'never saved'),
    doc_state.modified ? (doc_state.filename ? 'Not saved' : 'Modified') :
        'Not modified',
  ];
  this.icon = doc_state.icon;
  this.state = doc_state.state;
  this.title = doc_state.name;
  this.tooltip =  tooltip_texts.join('\n');
};

(function() {
  /**
   * @type {!Map.<!Window, !DocumentState>}
   */
  var tabStateMap = new Map();

  /**
   * TODO(yosi) Once we have "attach" document event, we should remove
   * |TabData.get()|.
   * @param {!Window} window
   * @return {?TabData}
   */
  global.TabData.get = function(window) {
    return tabStateMap.get(window);
  };

  /**
   * TODO(yosi) Once we have "attach" document event, we should remove
   * |TabData.update()|.
   * @param {!Window} window
   * @param {!Document} document
   * @return {?TabData}
   */
  global.TabData.update = function(window, document) {
    var state = DocumentState.get(document);
    if (!state)
      return;
    updateTabData(window, state);
  };

  /**
   * @param {!DocumentWindow} window
   * @param {!DocumentState} state
   */
  function updateTabData(window, state) {
    var tab_state = tabStateMap.get(window);
    if (tab_state && tab_state.equals(state))
      return;
    Editor.setTabData(window, new TabData(state));
    tabStateMap.set(window, state);
  }

  DocumentState.addObserver(function(document, state) {
    document.listWindows().forEach(function(window) {
      updateTabData(window, state);
    });
  });
})();
