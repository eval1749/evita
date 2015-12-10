// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'repl', function($export) {
  /** @const @type {string } */
  const DOCUMENT_NAME = '*javascript*';

  /** @return {!Document} */
  function ensureDocument() {
    const present = Document.find(DOCUMENT_NAME);
    if (present)
      return present;
    const newDocument = Document.new(DOCUMENT_NAME);
    newDocument.mode = Mode.chooseModeByFileName('foo.js');
    return newDocument;
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
      const document = this.ensureDocument_();
      const range = this.range_;
      range.collapseTo(0);
      range.end = document.length;
      const readonly = document.readonly;
      document.readonly = false;
      range.text = '';
      document.readonly = readonly;
      this.update();
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

    /** @return {!Document} */
    ensureDocument_() {
      if (this.document_)
        return this.document_;
      this.document_ = ensureDocument();
      this.range_ = new Range(this.document_);
      return this.document;
    }

    /**
     * Emits new line if console doesn't end with newline.
     */
    freshLine() {
      const document = this.ensureDocument_();
      if (document.length === 0)
        return;
      if (document.charCodeAt_(document.length - 1) === Unicode.LF)
        return;
      this.emit('\n');
    }

    /** @return {!Document} */
    get document() { return this.ensureDocument_(); }

    /**
     * @param {...*} params
     *
     *  Output arguments to console log.
     */
    log(...params) {
      const message = params.map(formatValue).join(' ');
      const document = this.ensureDocument_();
      const readonly = document.readonly;
      document.readonly = false;
      const range = this.range_;
      range.collapseTo(document.length);
      const start = range.start;
      range.startOf(Unit.LINE, Alter.EXTEND);
      if (start !== range.start) {
        range.collapseTo(range.end);
        range.insertBefore('\n');
      }
      range.insertBefore(message + '\n');
      document.readonly = readonly;
      this.update();
    }

    reset_() {
      this.document_ = null;
    }

    // Activate or create window to show console log.
    show() {
      const document = this.ensureDocument_();
      const windows = document.listWindows();
      if (windows.length) {
        windows[0].focus();
        return;
      }
      const editorWindow = new EditorWindow();
      const window = new TextWindow(/** @type {!Range} */(this.range_));
      editorWindow.appendChild(window);
      editorWindow.realize();
    }

    update() {
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
