// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'windows', function($export) {
  /**
   * @type {!Array<!TextDocumentStateCallback>}
   */
  const observers = [];

  /**
   * @type {!Map<!TextDocument, !TextDocumentState>}
   */
  const stateMap = new Map();

  //////////////////////////////////////////////////////////////////////
  //
  // TextDocumentState
  //
  class TextDocumentState {
    /**
     * @param {!TextDocument} document
     */
    constructor(document) {
      this.document_ = document;
      this.fileName = document.fileName;
      // TODO(eval1749): We should specify |TextDocumentState.prototype.icon| from
      // IconSet.
      this.icon = -2;
      this.lastWriteTime = document.lastWriteTime;
      this.modified = document.modified;
      this.name = document.name;
      this.state = 0;
    }

    /**
     * @private
     * Set initial tab data for |TextWindow|.
     * @param {!TextDocumentEvent} event
     */
    didAttachWindow(event) {
      if (event.view === null)
        return;
      TabData.update(event.view, this);
    }

    /**
     * @private
     * @param {!TextDocumentEvent} event
     */
    didLoadSaveTextDocument(event) {
      const document = this.document_;
      this.fileName = document.fileName;
      this.lastWriteTime = document.lastWriteTime;
      this.modified = document.modified;
      this.state = 0;
      this.notifyChange();
    }

    /**
     * @private
     * @param {!Array.<!TextMutationRecord>} mutations
     * @param {!TextMutationObserver} observer
     */
    mutationCallback(mutations, observer) {
      if (this.modified === this.document_.modified)
        return;
      this.modified = this.document_.modified;
      this.notifyChange();
    }

    /**
     * @private
     */
    notifyChange() {
      observers.forEach(observer => observer(this.document_, this));
    }

    /**
     * @private
     */
    start() {
      const document = this.document_;
      document.addEventListener(Event.Names.ATTACH,
                                this.didAttachWindow.bind(this));
      document.addEventListener(Event.Names.BEFORELOAD,
                                this.willLoadTextDocument.bind(this));
      document.addEventListener(Event.Names.LOAD,
                                this.didLoadSaveTextDocument.bind(this));
      document.addEventListener(Event.Names.SAVE,
                                this.didLoadSaveTextDocument.bind(this));
      const mutationObserver = new TextMutationObserver(
          this.mutationCallback.bind(this));
      mutationObserver.observe(document, {summary: true});
      this.notifyChange();
    }

    /**
     * @private
     * @param {!TextDocumentEvent} event
     */
    willLoadTextDocument(event) {
      this.state = 1;
      this.notifyChange();
    }

    /**
     * @param {!TextDocumentStateCallback} callback
     */
    static addObserver(callback) {
      observers.push(callback);
    }

    /**
     * For debugging only
     * @param {!TextDocument} document
     * @return {TextDocumentState}
     */
    static get(document) {
      return stateMap.get(document) || null;
    }

    /**
     * @param {!TextDocument} document
     */
    static startTracking(document) {
      const state = new TextDocumentState(document);
      stateMap.set(document, state);
      state.start();
    }
  }

  TextDocument.addObserver(function(type, document) {
    if (type === 'add')
      TextDocumentState.startTracking(document);
  });

  $export({TextDocumentState});
});

global.TextDocumentState = windows.TextDocumentState;
