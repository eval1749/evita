// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'repl', ($export) => {
  /**
   * @param {string} string
   * @return {*}
   */
  function evalAsContainer(string) {
    try {
      return eval(string);
    } catch (exception) {
      return undefined;
    }
  }

  /**
   * @param {*} container
   * @param {string} prefix
   * @return {!Set.<string>}
   *
   */
  function propertyNamesStartWith(container, prefix) {
    const names = new Set();
    if (container === null || container == undefined)
      return names;
    const predicate = prefix === ''
        ? name => !name.startsWith('_')
        : name => name.startsWith(prefix);
    prototypeChainOf(/** @type {!Object} */(container)).forEach(runner => {
      Object.getOwnPropertyNames(runner)
          .filter(predicate).forEach(name => names.add(name));
    });
    return names;
  }

  /**
   * @param {!Object} object
   * @return {!Array.<!Object>}
   */
  function prototypeChainOf(object) {
    let path = []
    for (let runner = object; runner;
         runner = Object.getPrototypeOf(runner)) {
      path.push(runner);
    }
    return path;
  }

  //////////////////////////////////////////////////////////////////////
  //
  // CompletionSession
  //
  class CompletionSession {
    /**
     * @param {!repl.JsConsole} commandLoop
     */
    constructor(commandLoop) {
      this.beforeText_ = '';
      /** @type {!Array.<string>} */
      this.candidates_ = [];
      /** @type {number} */
      this.index_ = 0;
      /** const @type {!repl.JsConsole} */
      this.repl_ = repl;
      /** @const @type {number} */
      this.lastLineStart_ = this.repl_.lastLineStart;
      /** @type {string} */
      this.lastLine_ = this.repl_.lastLine;
      this.setupCandiates_();
    }

    /** @return {boolean} */
    complete() {
      if (this.candidates_.length === 1)
        return false;
      this.lastLine_ = this.beforeText_ + this.candidates_[this.index_];
      this.repl_.lastLine = this.lastLine_;
      this.index_ = (this.index_ + 1) % this.candidates_.length;
      return true;
    }

    /** @return {boolean} */
    isAlive() {
      return this.lastLineStart_ === this.repl_.lastLineStart &&
             this.lastLine_ === this.repl_.lastLine;
    }

    /** @param {!Set.<string>} nameSet */
    setCandidates_(nameSet) {
      this.candidates_ = Array.from(nameSet).sort((a, b) => a.localeCompare(b));
    }

    setupCandiates_() {
      const line = this.lastLine_;
      if (line === '')
        return this.setCandidates_(new Set());
      const lastDotIndex = line.lastIndexOf('.');
      if (lastDotIndex < 0) {
        this.setCandidates_(propertyNamesStartWith(global, line));
        this.candidates_.push(line);
        return;
      }
      this.beforeText_ = line.substr(0, lastDotIndex + 1);
      const value = evalAsContainer(line.substr(0, lastDotIndex));
      const prefix = line.substr(lastDotIndex + 1);
      this.setCandidates_(propertyNamesStartWith(value, prefix));
      this.candidates_.push(prefix);
    }
  }

  /** @type {!Map.<!TextSelection, !JsCompleter>} */
  const completers = new Map();

  //////////////////////////////////////////////////////////////////////
  //
  // JsCompleter
  //
  class JsCompleter {
    /**
     * @param {!repl.JsConsole} commandLoop
     * @param {!TextSelection} selection
     */
    constructor(commandLoop, selection) {
      this.commandLoop_ = commandLoop;
      this.session_ = null;
      this.selection_ = selection;
    }

    // TODO(eval1749): Once closure compile recognizes local class declaration,
    // we should have type annotation.
    /* $return {!CompletionSession} */
    ensureSession_() {
      if (!this.session_ || !this.session_.isAlive())
        this.session_ = new CompletionSession(this.commandLoop_);
      return this.session_;
    }

    /** @return {boolean} */
    perform() {
      if (!this.ensureSession_().complete())
        return false;
      this.selection_.endOf(Unit.DOCUMENT);
      return true;
    }

    // TODO(eval1749): Once closure compile recognizes local class declaration,
    // we should have type annotation.
    /**
     * @param {!repl.JsConsole} commandLoop
     * @param {!TextSelection} selection
     * $return {!JsCompleter}
     */
    static ensure(commandLoop, selection) {
      const present = completers.get(selection);
      if (present)
        return present;
      const completer = new JsCompleter(commandLoop, selection);
      completers.set(selection, completer);
      return completer;
    }
  }

  $export({JsCompleter});
});
