// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('repl.JsConsole');

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
goog.scope(function() {
/** @const @type {string} */ const BLOCK_COMMENT = '\x2F*';
/** @const @type {string} */ const BLOCK_COMMENT_END = '*\x2F';
/** @const @type {string} */ const LINE_COMMENT = '\x2F/';
/** @const @type {number} */ const MAX_HISTORY_LINES = 20;

/** @const @type {!Map<string, !Object>} */
const keyBindings = new Map();

/**
 * @param {*} reason
 * @return {string}
 */
function formatReason(reason) {
  if (!(reason instanceof Error))
    return repl.stringify(reason);
  const error = /** @type {!Error} */ (reason);
  const lines = [`${error.name}: ${error.message}`];
  if ('sourceLine' in error)
    lines.push(`Source code: ${error['sourceLine']}`);
  if (('fileName' in error) && 'lineNumber' in error)
    lines.push(`Location: ${error['fileName']}(${error['lineNumber']})`);
  if ('stack' in error)
    lines.push(`Stack: ${error['stack']}`);
  return lines.map(line => ` * ${line}`).join('\n');
}

//////////////////////////////////////////////////////////////////////
//
// History
//
class History {
  constructor() {
    /** @type {number} */
    this.index_ = 0;
    /** @type {!Array<string>} */
    this.lines_ = [];
  }

  /** @return {string} */
  get current() { return this.lines_[this.lines_.length - this.index_]; }

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
  constructor() {
    // TODO(eval1749): We should annotate |history_| with |History| once
    // Closure compiler recognize class |History|.
    /** @const $type {!History} */
    this.history_ = new History();
    /** @type {number} */
    this.lineNumber_ = 0;
    /** @const @type {!RepeatingTimer} */
    this.promiseTimer_ = new RepeatingTimer();
    /** @const @type {!TextRange} */
    this.range_ = new TextRange(this.document);
    /** @const @type {!Array} */
    this.results_ = [];
    this.installKeyBindings();
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
    this.range_.collapseTo(this.document.length - 1);
    this.document.readonly = false;
  }

  evalLastLine() {
    const line = this.lastLine.trim();
    if (line === '') {
      this.emitPrompt();
      return;
    }
    this.history_.add(line);
    console.freshLine();
    try {
      const value = Editor.runScript(line, console.document.name);
      console.freshLine();
      if (value instanceof Promise) {
        this.rememberResult_(value);
        return this.handlePromiseResult(value);
      }
      if (value !== undefined) {
        this.rememberResult_(value);
        console.emit(repl.stringify(value));
      }
    } catch (reason) {
      this.rememberResult_(reason);
      console.freshLine();
      console.emit(BLOCK_COMMENT);
      console.emit('\n');
      console.emit(formatReason(reason));
      console.emit('\n');
      console.emit(BLOCK_COMMENT_END);
    }
    this.emitPrompt();
  }

  forwardHistory() {
    if (!this.history_.forward())
      return;
    this.useHistory();
  }

  /** @return {!TextDocument} */
  get document() { return console.document; }

  /** @return {string} */
  get lastLine() {
    return this.document.slice(this.lastLineStart, this.document.length);
  }

  /** @param {string} string */
  set lastLine(string) {
    this.range.endOf(Unit.DOCUMENT, Alter.EXTEND);
    this.range_.text = ' ' + string;
  }

  /** @return {number} */
  get lastLineStart() { return this.range_.start + 1; }

  /** @return {number} */
  get lineNumber() { return this.lineNumber_; }

  /** @return {!TextRange} */
  get range() { return this.range_; }

  /**
   * @private
   * @param {!Promise} promise
   */
  handlePromiseResult(promise) {
    console.freshLine();
    console.log(LINE_COMMENT, 'Waiting for', promise);
    /** @type {number} */
    let counter = 0;
    this.promiseTimer_.start(1000, () => {
      ++counter;
      console.log(LINE_COMMENT, 'waiting promise...', promise, counter);
    });
    promise
        .then((value) => {
          console.log(LINE_COMMENT, 'Value of', promise, 'is:');
          console.emit(repl.stringify(value));
          this.emitPrompt();
        })
        .catch((reason) => {
          console.freshLine();
          console.log(BLOCK_COMMENT, promise, 'is rejected with:');
          this.rememberResult_(reason);
          console.emit(formatReason(reason));
          console.log(BLOCK_COMMENT_END);
          this.emitPrompt();
        });
  }

  /** @private */
  installKeyBindings() {
    for (const key of keyBindings.keys()) {
      const command =
          /** @type {function(!TextWindow, number=)} */ (keyBindings.get(key));
      Editor.bindKey(this.document, key, command);
    }
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

  useHistory() { this.lastLine = this.history_.current; }

  /**
   * @param {string} keyCombination
   * @param {!Object} command
   */
  static bindKey(keyCombination, command) {
    keyBindings.set(keyCombination, command);
    Editor.bindKey(console.document, keyCombination, command);
  }

  /** @return {!JsConsole} */
  static get instance() {
    const document = console.document;
    const present = document.properties.get(JsConsole.name) || null;
    if (present)
      return /** @type {!JsConsole} */ (present);
    const newInstance = new JsConsole();
    document.properties.set(JsConsole.name, newInstance);
    return newInstance;
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

/** @constructor */
repl.JsConsole = JsConsole;
});

global.JsConsole = repl.JsConsole;
