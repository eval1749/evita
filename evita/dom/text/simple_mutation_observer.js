// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'text', function($export) {
  /** @const @type {!WeakMap.<!TextDocument, !TextDocumentObserver>} */
  const observerMap = new WeakMap();

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

      /** @type {!Set.<!text.SimpleMutationObserver>} */
      this.observers_ = new Set();

      this.observer_ = new TextMutationObserver(
          this.mutationCallback_.bind(this));
      this.startObserving_();
      document.addEventListener(Event.Names.BEFORELOAD,
                                this.willLoadTextDocument_.bind(this));
      document.addEventListener(Event.Names.LOAD,
                                this.didLoadTextDocument_.bind(this));
    }

    /**
     * @param {!text.SimpleMutationObserver} observer
     */
    add(observer) {
      this.observers_.add(observer);
    }

    didLoadTextDocument_() {
      this.startObserving_();
      for (let observer of this.observers_.values())
        observer.didLoadTextDocument();
    }

    /**
     * @param {!TextDocument} document
     */
    static getOrCreate(document) {
      const observer = observerMap.get(document);
      if (observer)
        return observer;
      const newObserver = new TextDocumentObserver(document);
      observerMap.set(document, newObserver);
      return newObserver;
    }

    /**
     * @private
     * @param {!Array.<!TextMutationRecord>} mutations
     * @param {!TextMutationObserver} observer
     *
     * Resets hot offset to minimal changed offset and kicks word scanner.
     */
    mutationCallback_(mutations, observer) {
      /** @type {number} */
      const offset = mutations.reduce((previousValue, mutation) => {
        return Math.min(previousValue, mutation.offset);
      }, this.document_.length);
      for (let observer of this.observers_.values())
        observer.didChangeTextDocument(offset);
    }

    /**
     * @param {!text.SimpleMutationObserver} observer
     */
    remove(observer) {
      this.observers_.delete(observer);
    }

    /** @private */
    startObserving_() {
      this.observer_.observe(this.document_, {summary: true});
    }

    /**
     * @private
     */
    willLoadTextDocument_() {
      this.observer_.disconnect();
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // SimpleMutationObserverBase
  //
  class SimpleMutationObserverBase {
    /**
     * @param {!TextDocument} document
     */
    constructor(document) {
      /** @type {!TextDocument} */
      this.document_ = document;
      TextDocumentObserver.getOrCreate(document).add(this);
    }

    /** @return {!TextDocument} */
    get document() { return this.document_; }

    didLoadTextDocument() {}

    /*
     * implements text.SimpleMutationObserver.stopObserving()
     */
    stopObserving() {
      TextDocumentObserver.getOrCreate(this.document_).remove(this);
    }
  }

  $export({SimpleMutationObserverBase});
});
