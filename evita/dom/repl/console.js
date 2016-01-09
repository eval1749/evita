// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'repl', function($export) {
  /** @const @type {string } */
  const DOCUMENT_NAME = '*javascript*';

  /** @return {!TextDocument} */
  function ensureTextDocument() {
    const present = TextDocument.find(DOCUMENT_NAME);
    if (present)
      return present;
    const newTextDocument = TextDocument.new(DOCUMENT_NAME);
    newTextDocument.mode = Mode.chooseModeByFileName('foo.js');
    return newTextDocument;
  }

  /**
   * @param {*} value
   * @return {string}
   */
  function formatValue(value) {
    if (typeof(value) === 'string')
      return value;
    try {
      return repl.stringify(value);
    } catch (exception) {
      return formatValue(exception);
    }
  }

  ////////////////////////////////////////////////////////////
  //
  // Console
  //
  class Console {
    constructor() {
      this.document_ = null;
      this.range_ = null;
    }

    /**
     * @param {*} expression
     * @param {...*} params
     */
    assert(expression, ...params) {
      if (expression)
        return;
      if (params.length === 0)
        throw new Error('Assertion failed');
      const message = params.map(formatValue).join(' ');
      throw new Error('Assertion failed: ' + message);
    }

    /*
     * Clear console log contents.
     */
    clear() {
      const document = this.ensureTextDocument_();
      const range = this.range_;
      range.collapseTo(0);
      range.end = document.length;
      const readonly = document.readonly;
      document.readonly = false;
      range.text = '';
      document.readonly = readonly;
      this.update_();
    }

    /**
     * @param {string} text
     */
    emit(text) {
      this.document_.readonly = false;
      this.range_.endOf(Unit.DOCUMENT);
      this.range_.insertBefore(text);
      this.document_.readonly = true;
    }

    /**
     * @private
     * @return {!TextDocument}
     */
    ensureTextDocument_() {
      if (this.document_)
        return this.document_;
      this.document_ = ensureTextDocument();
      this.range_ = new Range(this.document_);
      return this.document;
    }

    /**
     * Emits new line if console doesn't end with newline.
     */
    freshLine() {
      const document = this.ensureTextDocument_();
      if (document.length === 0)
        return;
      if (document.charCodeAt(document.length - 1) === Unicode.LF)
        return;
      this.emit('\n');
    }

    /** @return {!TextDocument} */
    get document() { return this.ensureTextDocument_(); }

    /**
     * @param {...*} params
     *
     *  Output arguments to console log.
     */
    log(...params) {
      this.freshLine();
      this.emit(params.map(formatValue).join(' '));
      this.emit('\n');
      this.update_();
    }

    /**
     * @private
     * Disconnect console instance from document. This method is used for
     * resetting console output during testing.
     */
    reset_() {
      this.document_ = null;
      this.range_ = null;
    }

    /**
     * @private
     * Updates windows showing console.
     */
    update_() {
      for (const window of this.document_.listWindows())
        window.update();
    }
  }

  Object.defineProperty(Console, 'DOCUMENT_NAME', {value: DOCUMENT_NAME});

  const console = new Console();

  $initialize(function() {
    console.reset_();
  });

  $export({Console, console});
});

global.console = repl.console;
