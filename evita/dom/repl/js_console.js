// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var $0;
var $1;
var $2;

/**
 * The JavaScript Console.
 *
 * This file introduce editor command |SwitchToJavaScriptConsole| bound to
 * |Ctrl+Shift+I| and |Ctrl+Shift+J|.
 *
 * JavaScript console is represented by the document named |*javascript*| with
 * following key bindings:
 *  Enter Executes the last line, between end of prompt to end of document, as
 *        JavaScript source code and inserts result value into the document.
 *  Ctrl+Down Forward history
 *  Ctrl+L Clear console.
 *  Ctlr+Up Backward history
 *
 */
$define(global, 'repl', function($export) {
  /** @const @type {string} */ const BLOCK_COMMENT = '\x2F*';
  /** @const @type {string} */ const BLOCK_COMMENT_END = '*\x2F';
  /** @const @type {string} */ const LINE_COMMENT = '\x2F/';
  /** @const @type {number} */ const MAX_HISTORY_LINES = 20;

  /** @const @type {!Map.<string, !Object>} */
  const keyBindings = new Map();

  /** @type {JsConsole} */
  let staticInstance = null;

  /**
   * @param {*} reason
   * @return {string}
   */
  function formatReason(reason) {
    if (reason instanceof Error) {
      const stack = reason['stack'];
      if (stack)
        return stack.toString();
      const message = reason['message'];
      if (message) {
        return 'Exception: ' + message;
      }
    }
    return repl.stringify(reason);
  }

  //////////////////////////////////////////////////////////////////////
  //
  // History
  //
  class History {
    constructor() {
      /** @type {number} */
      this.index_ = 0;
      /** @type {!Array.<string>} */
      this.lines_ = [];
    }

    /** @return {string} */
    get current() {
      return this.lines_[this.lines_.length - this.index_];
    }

    /**
     * @param {string} line
     */
    add(line) {
      this.index_ = 0;
      if (!this.shouldAdd_(line))
        return;
      if (this.lines_.length > MAX_HISTORY_LINES)
        this.lines_.shift();
      this.lines_.push(line);
    }

    /** @return {boolean} */
    backward() {
      if (this.index_ === this.lines_.length)
        return false;
      ++this.index_;
      return true;
    }

    /** @return {boolean} */
    forward() {
      if (this.index_ === 0)
        return false;
      --this.index_;
      return true;
    }

    /**
     * @param {string} line
     * @return {boolean}
     */
    shouldAdd_(line) {
      const length = this.lines_.length;
      if (length === 0)
        return true;
      return this.lines_[length - 1] !== line;
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // JsConsole
  //
  class JsConsole {
    /** @param {!TextDocument} document */
    constructor(document) {
      /** @type {!TextDocument} */
      this.document_ = document;
      // TODO(eval1749): We should annotate |history_| with |History| once
      // Closure compiler recognize class |History|.
      /** @const $type {!History} */
      this.history_ = new History();
      /** @type {number} */
      this.lineNumber_ = 0;
      /** @const @type {!RepeatingTimer} */
      this.promiseTimer_ = new RepeatingTimer();
      /** @const @type {!Range} */
      this.range_ = new Range(document);
      /** @const @type {!Array} */
      this.results_ = [];

      for (const [key, command] of keyBindings.entries())
        document.bindKey(key, command);

      staticInstance = this;
    }

    backwardHistory() {
      if (!this.history_.backward())
        return;
      this.useHistory();
    }

    emitPrompt() {
      this.promiseTimer_.stop();
      ++this.lineNumber_;
      console.freshLine();
      console.emit('js:' + this.lineNumber_ + '> ');
      // Off by one to keep |this.range_.start| before user input.
      this.range_.collapseTo(this.document_.length - 1);
      this.document_.readonly = false;
    }

    evalLastLine() {
      const line = this.lastLine.trim();
      if (line === '') {
        this.emitPrompt();
        return;
      }
      this.history_.add(line);
      console.freshLine();
      const result = Editor.runScript(line, console.document.name);
      if (result.exception) {
        this.rememberResult_(result.exception);
        if (result.stackTraceString === '') {
          result.stackTraceString = result.exception +
            result.stackTrace.map(function(stackFrame) {
              return '  at ' + stackFrame.functionName + ' (' +
                  stackFrame.scriptName + '(' + stackFrame.lineNumber + ':' +
                  stackFrame.column + ')';
            }).join('\n');
        }
        console.freshLine();
        console.emit(BLOCK_COMMENT);
        console.emit('\nException: ');
        console.emit(result.stackTraceString);
        console.emit('\n');
        console.emit(BLOCK_COMMENT_END);
        this.emitPrompt();
        return;
      }

      const value = result.value;
      if (value instanceof Promise) {
        this.rememberResult_(value);
        return this.handlePromiseResult(value);
      }

      if (value !== undefined) {
        this.rememberResult_(value);
        console.freshLine();
        console.emit(repl.stringify(value));
      }
      this.emitPrompt();
    }

    forwardHistory() {
      if (!this.history_.forward())
        return;
      this.useHistory();
    }

    /** @return {!TextDocument} */
    get document() { return this.document_; }

    /** @return {string} */
    get lastLine() {
      return this.document_.slice(this.lastLineStart, this.document_.length);
    }

    /** @param {string} string */
    set lastLine(string) {
      this.range.endOf(Unit.DOCUMENT, Alter.EXTEND);
      this.range_.text = ' ' + string;
    }

    /** @return {number} */
    get lastLineStart() { return this.range_.start + 1; }

    /** @return {!Range} */
    get range() { return this.range_; }

    /**
     * @private
     * @param {!Promise} promise
     */
    handlePromiseResult(promise) {
      console.freshLine();
      console.log(LINE_COMMENT, 'Waiting for', promise);
      let counter = 0;
      this.promiseTimer_.start(1000, () => {
        ++counter;
        console.log(LINE_COMMENT, 'waiting promise...', promise, counter);
      });
      promise.then((value) => {
        console.log(LINE_COMMENT, 'Value of', promise, 'is:');
        console.emit(repl.stringify(value));
        this.emitPrompt();
      }).catch((reason) => {
        console.freshLine();
        console.log(BLOCK_COMMENT, promise, 'is rejected with:');
        this.rememberResult_(reason);
        console.emit(formatReason(reason));
        console.log(BLOCK_COMMENT_END);
        this.emitPrompt();
      });
    }

    /** @param {*} result */
    rememberResult_(result) {
      $2 = $1;
      $1 = $2;
      $0 = result;
      if (this.results_.length >= 10)
        this.results_.shift();
      this.results_.push($0);
    }

    /**
     * @param {string} keyCombination
     * @param {!Object} command
     */
    static bindKey(keyCombination, command) {
      if (!staticInstance) {
        keyBindings.set(keyCombination, command);
        return;
      }
      staticInstance.document.bindKey(keyCombination, command);
    }

    /** @return {!JsConsole} */
    static get instance() {
      if (!staticInstance)
        throw new Error('JsConsole is not initialized.');
      return staticInstance;
    }

    useHistory() {
      this.lastLine = this.history_.current;
    }
  }

  /**
   * @param {!Promise} promise
   * @param {*} reason
   * @param {number} event
   *
   * enum PromiseRejectEvent {
   *   kPromiseRejectWithNoHandler = 0,
   *   kPromiseHandlerAddedAfterReject = 1
   * };
   *
   * This function is called from C++ via
   * |v8::Isolate::SetPromiseRejectCallback()|.
   */
  function handleRejectedPromise(promise, reason, event) {
    console.freshLine();
    console.log(BLOCK_COMMENT, 'Unhandled promise rejection:', event);
    console.emit(formatReason(reason));
    console.log(BLOCK_COMMENT_END);
  }

  Editor.handleRejectedPromise = handleRejectedPromise;

  $export({JsConsole});
});

global.JsConsole = repl.JsConsole;
