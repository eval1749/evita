// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('repl.Console');

goog.scope(function() {

const Initializer = core.Initializer;

/** @const @type {string } */
const kDocumentName = '*javascript*';

/** @return {!TextDocument} */
function ensureTextDocument() {
  /** @const @type {TextDocument} */
  const present = TextDocument.find(kDocumentName);
  if (present)
    return present;
  /** @const @type {!TextDocument} */
  const newTextDocument = TextDocument.new(kDocumentName);
  newTextDocument.dispatchEvent(new TextDocumentEvent(Event.Names.NEWFILE));
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
    /** @type {TextDocument} */
    this.document_ = null;
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
    /** @const @type {string} */
    const message = params.map(formatValue).join(' ');
    throw new Error(`Assertion failed: ${message}`);
  }

  /*
   * Clear console log contents.
   */
  clear() {
    /** @const @type {!TextDocument} */
    const document = this.ensureTextDocument();
    /** @const @type {boolean} */
    const readonly = document.readonly;
    document.readonly = false;
    document.replace(0, document.length, '');
    document.readonly = readonly;
    this.update_();
  }

  /**
   * @param {string} text
   */
  emit(text) {
    /** @const @type {!TextDocument} */
    const document = this.ensureTextDocument();
    document.readonly = false;
    document.replace(document.length, document.length, text);
    document.readonly = true;
  }

  /**
   * @private
   * @return {!TextDocument}
   */
  ensureTextDocument() {
    if (this.document_)
      return this.document_;
    this.document_ = ensureTextDocument();
    this.document_.addEventListener(
        Event.Names.REMOVE, () => { this.document_ = null; });
    return this.document;
  }

  /**
   * Emits new line if console doesn't end with newline.
   */
  freshLine() {
    /** @const @type {!TextDocument} */
    const document = this.ensureTextDocument();
    if (document.length === 0)
      return;
    if (document.charCodeAt(document.length - 1) === Unicode.LF)
      return;
    this.emit('\n');
  }

  /** @return {!TextDocument} */
  get document() { return this.ensureTextDocument(); }

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
  reset_() { this.document_ = null; }

  /**
   * @private
   * Updates windows showing console.
   */
  update_() {
    for (/** @type {!Window} */ const window of this.document_.listWindows())
      window.update();
  }
}

/** @const @type {!Console} */
const console = new Console();

Initializer.register(() => console.reset_());

/** @constructor */
repl.Console = Console;
/** @type {!Console} */
repl.console = console;
});

global.console = repl.console;
