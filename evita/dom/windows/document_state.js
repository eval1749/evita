// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'windows', function($export) {
  /**
   * @type {!Array<!DocumentStateCallback>}
   */
  const observers = [];

  /**
   * @type {!Map<!Document, !DocumentState>}
   */
  const stateMap = new Map();

  //////////////////////////////////////////////////////////////////////
  //
  // DocumentState
  //
  class DocumentState {
    /**
     * @param {!Document} document
     */
    constructor(document) {
      this.document_ = document;
      this.fileName = document.fileName;
      // TODO(eval1749): We should specify |DocumentState.prototype.icon| from
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
     * @param {!DocumentEvent} event
     */
    didAttachWindow(event) {
      if (event.view === null)
        return;
      TabData.update(event.view, this);
    }

    /**
     * @private
     * @param {!DocumentEvent} event
     */
    didLoadSaveDocument(event) {
      const document = this.document_;
      this.fileName = document.fileName;
      this.lastWriteTime = document.lastWriteTime;
      this.modified = document.modified;
      this.state = 0;
      this.notifyChange();
    }

    /**
     * @private
     * @param {!Array.<!MutationRecord>} mutations
     * @param {!MutationObserver} observer
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
                                this.willLoadDocument.bind(this));
      document.addEventListener(Event.Names.LOAD,
                                this.didLoadSaveDocument.bind(this));
      document.addEventListener(Event.Names.SAVE,
                                this.didLoadSaveDocument.bind(this));
      const mutationObserver = new MutationObserver(
          this.mutationCallback.bind(this));
      mutationObserver.observe(document, {summary: true});
      this.notifyChange();
    }

    /**
     * @private
     * @param {!DocumentEvent} event
     */
    willLoadDocument(event) {
      this.state = 1;
      this.notifyChange();
    }

    /**
     * @param {!DocumentStateCallback} callback
     */
    static addObserver(callback) {
      observers.push(callback);
    }

    /**
     * For debugging only
     * @param {!Document} document
     * @return {DocumentState}
     */
    static get(document) {
      return stateMap.get(document) || null;
    }

    /**
     * @param {!Document} document
     */
    static startTracking(document) {
      const state = new DocumentState(document);
      stateMap.set(document, state);
      state.start();
    }
  }

  Document.addObserver(function(type, document) {
    if (type === 'add')
      DocumentState.startTracking(document);
  });

  $export({DocumentState});
});

global.DocumentState = windows.DocumentState;
