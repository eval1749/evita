// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'windows', function($export) {
  //////////////////////////////////////////////////////////////////////
  //
  // TabData
  //
  class TabData {
    constructor() {
      this.icon = 0;
      this.state = 0;
      this.title = '?';
      this.tooltip = '';
    }

    /**
     * @param {!TextDocumentState} state
     * @return {!TabData}
     */
    static newTabDataFromState(state) {
      const tabData = new TabData();
      const tooltip_texts = [
        'Name: ' + state.name,
        'File: ' + (state.fileName ? state.fileName : 'no file'),
        'Save: ' + (state.lastWriteTime.valueOf() ?
            state.lastWriteTime.toLocaleString() : 'never saved'),
        state.modified ? (state.fileName ? 'Not saved' : 'Modified') :
            'Not modified',
      ];
      tabData.icon = state.icon;
      tabData.state = state.modified ? 1 : 0;
      tabData.title = state.name;
      tabData.tooltip =  tooltip_texts.join('\n');
      return tabData;
    }

    /**
     * This function is called when document is attached to window.
     *
     * @param {!Window} window
     * @param {!TextDocumentState} state
     */
    static update(window, state) {
      if (!(window instanceof TextWindow))
        return;
      Editor.setTabData(window, TabData.newTabDataFromState(state));
    }
  }

  /**
   * Updates tab data for windows attached to |document| with new |state| when
   * |TextDocumentState| is changed.
   *
   * @param {!TextDocument} document
   * @param {!TextDocumentState} state
   */
  function observer(document, state) {
    document.listWindows().forEach(window => TabData.update(window, state));
  }
  TextDocumentState.addObserver(observer);

  $export({TabData});
});

global.TabData = windows.TabData;
