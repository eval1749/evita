// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!Document|!DocumentState} document_or_state
 */
global.DocumentState = function(document_or_state) {
  if (document_or_state instanceof Document) {
    var document = /** @type {!Document} */(document_or_state);
    this.fileName = document.fileName;
    // TODO(yosi) We should specify |DocumentState.prototype.icon| from
    // IconSet.
    this.icon = -2;
    this.lastWriteTime = document.lastWriteTime;
    this.modified = document.modified;
    this.name = document.name;
    this.state = document.state;
    return;
  }

  if (document_or_state instanceof DocumentState) {
    var state = document_or_state;
    this.fileName = state.fileName;
    this.icon = state.icon;
    this.lastWriteTime = state.lastWriteTime;
    this.modified = state.modified;
    this.name = state.name;
    this.state = state.state;
    return;
  }

  throw new TypeError('Expect Document or DocumentState');
};

/**
 * @this {!DocumentState}
 * @return {!DocumentState}
 */
global.DocumentState.prototype.clone = function() {
  return new DocumentState(this);
};

/**
 * @this {!DocumentState}
 * @param {!DocumentState} other
 * @return {boolean}
 */
global.DocumentState.prototype.equals = function(other) {
  return this.fileName == other.fileName &&
         this.icon == other.icon &&
         this.lastWriteTime == other.lastWriteTime &&
         this.modified == other.modified &&
         this.name == other.name &&
         this.state == other.state;
};

(function() {
  /**
   * @type {!Map.<!Document, !DocumentState>}
   */
  var documentStateMap = new Map();

  /**
   * @type {!Array.<!DocumentStateCallback>}
   */
  var observers = [];

  /**
   * @type {!function(!Document)} callback
   */
  global.DocumentState.addObserver = function(callback) {
    observers.push(callback);
  };

  /**
   * For testing purpose only.
   * @param {!Document} document
   * @return {!DocumentState}
   */
  global.DocumentState.get = function(document) {
    return documentStateMap.get(document);
  };

  /**
   * Updates document state by |document.modified| property. This function is
   * intended to be called from "idle" event handler to update |modified|
   * property of |DocumentState|.
   *
   * @param {!Document} document
   */
  global.DocumentState.update = function(document) {
    var state = documentStateMap.get(document);
    if (!state)
      return;
    if (state.modified == document.modified)
      return;
    state.modified = document.modified;
    observers.forEach(function(observer) {
      observer.call(this, document, state);
    });
  };

  /**
   * Set initial tab data for |TextWindow|.
   * @param {!DocumentEvent} event
   */
  function didAttachWindow(event) {
    if (!(event.view instanceof TextWindow))
      return;
    var document = /** @type{!Document} */(event.target);
    var state = documentStateMap.get(document);
    if (!state)
      return;
    TabData.update(event.view, state);
  }

  /**
   * @param {!DocumentEvent} event
   */
  function didDocumentLoadSave(event) {
    var document = /** @type{!Document} */(event.target);
    var state = new DocumentState(document);
    documentStateMap.set(documentStateMap, state);
    observers.forEach(function(observer) {
      observer.call(this, document, state);
    });
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
    var document = /** @type{!Document} */(event.target);
    var state = documentStateMap.get(document);
    if (!state)
      return;
    state.state = 1;
    observers.forEach(function(observer) {
      observer.call(this, document, state);
    });
  }

  Document.addObserver(function(type, document) {
    if (type == 'add')
      startTracking(document);
  });
})();
