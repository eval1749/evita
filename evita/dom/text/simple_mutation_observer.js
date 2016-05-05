// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {
/** @const @type {!WeakMap.<!TextDocument, !TextDocumentObserver>} */
const observerMap = new WeakMap();

/**
 * @interface
 */
var SimpleMutationObserver = function() {};

/**
 * @param {number} headCount
 * @param {number} tailCount
 * @param {number} delta
 */
SimpleMutationObserver.prototype.didChangeTextDocument = function(
    headCount, tailCount, delta) {};

SimpleMutationObserver.prototype.didLoadTextDocument = function() {};

//////////////////////////////////////////////////////////////////////
//
// TextDocumentObserver
//
class TextDocumentObserver {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    /** @const @type {!TextDocument} */
    this.document_ = document;

    /** @type {!Set<!SimpleMutationObserver>} */
    this.clients_ = new Set();

    /** @const @type {!TextMutationObserver} */
    this.observer_ =
        new TextMutationObserver(this.mutationCallback_.bind(this));
    this.startObserving_();
    document.addEventListener(
        Event.Names.BEFORELOAD, this.willLoadTextDocument_.bind(this));
    document.addEventListener(
        Event.Names.LOAD, this.didLoadTextDocument_.bind(this));
  }

  /**
   * @param {!SimpleMutationObserver} client
   */
  add(client) { this.clients_.add(client); }

  /** @private */
  didLoadTextDocument_() {
    this.startObserving_();
    for (const client of this.clients_.values())
      client.didLoadTextDocument();
  }

  /**
   * @private
   * @param {!Array<!TextMutationRecord>} mutations
   * @param {!TextMutationObserver} observer
   *
   * Resets hot offset to minimal changed offset and kicks word scanner.
   */
  mutationCallback_(mutations, observer) {
    if (mutations.length === 0)
      return;
    /** @const @type {!TextMutationRecord} */
    const mutation = mutations[0];
    /** @const @type {number} */
    const headCount = mutation.headCount;
    /** @const @type {number} */
    const tailCount = mutation.tailCount;
    /** @const @type {number} */
    const delta = mutation.delta;
    for (const client of this.clients_.values())
      client.didChangeTextDocument(headCount, tailCount, delta);
  }

  /**
   * @param {!SimpleMutationObserver} client
   */
  remove(client) { this.clients_.delete(client); }

  /** @private */
  startObserving_() { this.observer_.observe(this.document_, {summary: true}); }

  /** @private */
  willLoadTextDocument_() { this.observer_.disconnect(); }

  /**
   * @param {!TextDocument} document
   * @return {!TextDocumentObserver}
   */
  static getOrCreate(document) {
    /** @const @type {!TextDocumentObserver|undefined} */
    const observer = observerMap.get(document);
    if (observer)
      return observer;
    /** @const @type {!TextDocumentObserver} */
    const newObserver = new TextDocumentObserver(document);
    observerMap.set(document, newObserver);
    return newObserver;
  }
}

/**
 * @implements {SimpleMutationObserver}
 */
class SimpleMutationObserverBase {
  /**
   * @protected
   * @param {!TextDocument} document
   */
  constructor(document) {
    /** @type {!TextDocument} */
    this.document_ = document;
    TextDocumentObserver.getOrCreate(document).add(this);
  }

  /**
   * @public
   * @return {!TextDocument}
   */
  get document() { return this.document_; }

  /**
   * @public
   * @param {number} headCount
   * @param {number} tailCount
   * @param {number} delta
   */
  didChangeTextDocument(headCount, tailCount, delta) {
    /** @const @type {string} */
    const name = this.constructor.name;
    throw new Error(`${name} should implement didChangeTextDocument()`);
  }

  /**
   * @public
   * Called by |TextDocumentObserver.protected.mutationCallback_()|.
   */
  didLoadTextDocument() {
    /** @const @type {string} */
    const name = this.constructor.name;
    throw new Error(`${name} should implement didLoadTextDocument()`);
  }

  /**
   * @public
   * implements text.SimpleMutationObserver.stopObserving()
   */
  stopObserving() {
    TextDocumentObserver.getOrCreate(this.document_).remove(this);
  }
}

/** @constructor */
text.SimpleMutationObserverBase = SimpleMutationObserverBase;

/** @interface */
text.SimpleMutationObserver = SimpleMutationObserver;
});
