// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'text', function($export) {
  /** @const @type {!WeakMap.<!Document, !DocumentObserver>} */
  const observerMap = new WeakMap();

  //////////////////////////////////////////////////////////////////////
  //
  // DocumentObserver
  //
  class DocumentObserver {
    /**
     * @param {!Document} document
     */
    constructor(document) {
      /** @const @type {!Document} */
      this.document_ = document;

      /** @type {!Set.<!text.SimpleMutationObserver>} */
      this.observers_ = new Set();

      this.observer_ = new MutationObserver(
          this.mutationCallback_.bind(this));
      this.startObserving_();
      document.addEventListener(Event.Names.BEFORELOAD,
                                this.willLoadDocument_.bind(this));
      document.addEventListener(Event.Names.LOAD,
                                this.didLoadDocument_.bind(this));
    }

    /**
     * @param {!text.SimpleMutationObserver} observer
     */
    add(observer) {
      this.observers_.add(observer);
    }

    didLoadDocument_() {
      this.startObserving_();
      for (let observer of this.observers_.values())
        observer.didLoadDocument();
    }

    /**
     * @param {!Document} document
     */
    static getOrCreate(document) {
      const observer = observerMap.get(document);
      if (observer)
        return observer;
      const newObserver = new DocumentObserver(document);
      observerMap.set(document, newObserver);
      return newObserver;
    }

    /**
     * @private
     * @param {!Array.<!MutationRecord>} mutations
     * @param {!MutationObserver} observer
     *
     * Resets hot offset to minimal changed offset and kicks word scanner.
     */
    mutationCallback_(mutations, observer) {
      /** @type {number} */
      const offset = mutations.reduce((previousValue, mutation) => {
        return Math.min(previousValue, mutation.offset);
      }, this.document_.length);
      for (let observer of this.observers_.values())
        observer.didChangeDocument(offset);
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
    willLoadDocument_() {
      this.observer_.disconnect();
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // SimpleMutationObserverBase
  //
  class SimpleMutationObserverBase {
    /**
     * @param {!Document} document
     */
    constructor(document) {
      /** @type {!Document} */
      this.document_ = document;
      DocumentObserver.getOrCreate(document).add(this);
    }

    /** @return {!Document} */
    get document() { return this.document_; }

    didLoadDocument() {}

    /*
     * implements text.SimpleMutationObserver.stopObserving()
     */
    stopObserving() {
      DocumentObserver.getOrCreate(this.document_).remove(this);
    }
  }

  $export({SimpleMutationObserverBase});
});
