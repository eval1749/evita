// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'windows', function($export) {
  /**
   * @type {!Map.<!Document, !DocumentState>}
   */
  const documentStateMap = new Map();

  /**
   * @type {!Array.<!DocumentStateCallback>}
   */
  const observers = [];

  //////////////////////////////////////////////////////////////////////
  //
  // DocumentState
  //
  class DocumentState {
    /**
     * @param {!Document|!DocumentState} document_or_state
     */
    constructor(document_or_state) {
      if (document_or_state instanceof Document) {
        const document = /** @type {!Document} */(document_or_state);
        this.fileName = document.fileName;
        // TODO(eval1749): We should specify |DocumentState.prototype.icon| from
        // IconSet.
        this.icon = -2;
        this.lastWriteTime = document.lastWriteTime;
        this.modified = document.modified;
        this.name = document.name;
        this.state = 0;
        return;
      }

      if (document_or_state instanceof DocumentState) {
        const state = document_or_state;
        this.fileName = state.fileName;
        this.icon = state.icon;
        this.lastWriteTime = state.lastWriteTime;
        this.modified = state.modified;
        this.name = state.name;
        this.state = state.state;
        return;
      }

      throw new TypeError('Expect Document or DocumentState');
    }

    /**
     * @return {!DocumentState}
     */
    clone() {
      return new DocumentState(this);
    }

    /**
     * @param {!DocumentState} other
     * @return {boolean}
     */
    equals(other) {
      return this.fileName === other.fileName &&
             this.icon === other.icon &&
             this.lastWriteTime === other.lastWriteTime &&
             this.modified === other.modified &&
             this.name === other.name &&
             this.state === other.state;
    }

    /**
     * @param {!DocumentStateCallback} callback
     */
    static addObserver(callback) {
      observers.push(callback);
    }

    /**
     * For testing purpose only.
     * @param {!Document} document
     * @return {DocumentState}
     */
    static get(document) {
      return documentStateMap.get(document) || null;
    }

    /**
     * Updates document state by |document.modified| property. This function is
     * intended to be called from "idle" event handler to update |modified|
     * property of |DocumentState|.
     *
     * @param {!Document} document
     */
    static update(document) {
      const maybe_state = documentStateMap.get(document) || null;
      if (maybe_state === null)
        return;
      const state = /** @type {!DocumentState} */(maybe_state);
      if (state.modified === document.modified)
        return;
      state.modified = document.modified;
      observers.forEach(observer => observer(document, state));
    }
  }

  /**
   * Set initial tab data for |TextWindow|.
   * @param {!DocumentEvent} event
   */
  function didAttachWindow(event) {
    if (!(event.view instanceof TextWindow))
      return;
    const document = /** @type{!Document} */(event.target);
    const maybe_state = documentStateMap.get(document) || null;
    if (maybe_state === null)
      return;
    const state = /** @type {!DocumentState} */(maybe_state);
    TabData.update(event.view, state);
  }

  /**
   * @param {!DocumentEvent} event
   */
  function didDocumentLoadSave(event) {
    const document = /** @type{!Document} */(event.target);
    const state = new DocumentState(document);
    documentStateMap.set(document, state);
    observers.forEach(observer => observer(document, state));
  }

  /**
   * @param {!Document} document
   */
  function startTracking(document) {
    documentStateMap.set(document, new DocumentState(document));
    document.addEventListener(Event.Names.ATTACH, didAttachWindow);
    document.addEventListener(Event.Names.BEFORELOAD, willLoadDocument);
    document.addEventListener(Event.Names.LOAD, didDocumentLoadSave);
    document.addEventListener(Event.Names.SAVE, didDocumentLoadSave);
  }

  /**
   * @param {!DocumentEvent} event
   */
  function willLoadDocument(event) {
    const document = /** @type{!Document} */(event.target);
    const maybe_state = documentStateMap.get(document) || null;
    if (maybe_state === null)
      return;
    const state = /** @type {!DocumentState} */(maybe_state);
    state.state = 1;
    observers.forEach(observer => observer(document, state));
  }

  Document.addObserver(function(type, document) {
    if (type === 'add')
      startTracking(document);
  });

  $export({DocumentState});
});

global.DocumentState = windows.DocumentState;
