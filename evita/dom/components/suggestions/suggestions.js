// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Note: Session should be in file scope. Although, when we use
// IIFE, immediately-invoked function expression, Closure compiler can't find
// |Session| type.

goog.provide('suggestions');

goog.require('base.Logger');

goog.scope(function() {

const Logger = base.Logger;

/** @const @type {!Map<!TextDocument, !Session>} */
const sessionMap = new Map();

/**
 * @param {string} text
 * @param {!TextDocument} document
 * @param {number} offset
 * @return {boolean}
 */
function matchAt(text, document, offset) {
  for (/** @type {number} */ let k = 0; k < text.length; ++k) {
    if (document.charCodeAt(offset) !== text.charCodeAt(k))
      return false;
    ++offset;
  }
  return true;
}

/**
 * @param {string} text
 * @param {!TextRange} range
 * @param {number} direction
 * @return {boolean}
 */
function findWordStartsWith(text, range, direction) {
  console.assert(text !== '');
  console.assert(direction !== 0);
  /** @const @type {!TextDocument} */
  const document = range.document;
  /** @const @type {number} */
  const textLength = text.length;
  if (direction > 0) {
    /** @const @type {number} */
    const maxStart = document.length - textLength;
    do {
      range.move(Unit.WORD, 1);
      if (range.start > maxStart)
        break;
      if (matchAt(text, document, range.start))
        return true;
    } while (range.start < maxStart);
    range.collapseTo(document.length);
    return false;
  }
  if (range.start === 0)
    return false;
  do {
    range.move(Unit.WORD, -1);
    if (range.start + textLength > document.length)
      break;
    if (matchAt(text, document, range.start))
      return true;
  } while (range.start);
  range.collapseTo(0);
  return false;
}

class Session extends Logger {
  /**
   * @public
   * @param {!TextDocument} document
   */
  constructor(document) {
    super();
    /** @const @type {!TextRange} */
    this.cursor = new TextRange(document);
    /** @type {number} */
    this.direction_ = 0;
    /** @type {number} */
    this.index_ = 0;
    /** @type {TextRange} */
    this.lastSelectionRange = null;
    /** @type {number} */
    this.lastSelectionStart = 0;
    /** @type {string} */
    this.prefix = '';
    /** @type {!Array<string>} */
    this.suggestions_ = [];
  }

  /**
   * @public
   * @param {!TextRange} range
   * @return {string}
   */
  expand(range) {
    if (!range.collapsed)
      return '';
    this.startOrContinue(range);
    if (this.suggestions_.length === this.index_) {
      if (!this.updateSuggetions()) {
        if (this.suggestions_.length === 0)
          return `No word starts with "${this.prefix}"`;
        if (this.suggestions_.length === 1)
          return `No more words start with "${this.prefix}"`;
        this.index_ = 0;
      }
    }
    /** @const @type {string} */
    const suggestion = this.suggestions_[this.index_];
    ++this.index_;
    range.text = suggestion;
    range.collapseTo(range.end);
    this.lastSelectionStart = range.end;
    return '';
  }

  /**
   * @param {!TextRange} range
   * @return {boolean} False if we should start new session.
   * TODO(eval1749): Check this command and last command
   */
  prepare(range) {
    if (this.prefix === '')
      return false;
    if (this.lastSelectionRange !== range) {
      // Someone invokes session in different window.
      return false;
    }
    if (this.lastSelectionStart !== range.start) {
      // Selection is moved.
      return false;
    }
    // selection is at end of expanded word.
    range.moveStart(Unit.WORD, -1);
    if (!range.text.startsWith(this.prefix)) {
      range.collapseTo(range.end);
      return false;
    }
    return true;
  }

  /**
   * @private
   * @param {!TextRange} range
   */
  startOrContinue(range) {
    if (this.prepare(range))
      return;
    this.cursor.collapseTo(range.start);
    this.cursor.moveStart(Unit.WORD, -1);
    this.direction_ = -1;
    this.prefix = this.cursor.text;
    this.lastSelectionRange = range;
    this.lastSelectionStart = range.start;
    this.suggestions_ = [];
    this.index_ = 0;
    range.start = this.cursor.start;
  }

  /** @override */
  toString() { return `suggestions.Session(${this.cursor})`; }

  /**
   * @private
   * @param {number} direction
   * @return {boolean}
   */
  tryAddSuggestion(direction) {
    /** @type {!TextRange} */
    const cursor = this.cursor;
    while (findWordStartsWith(this.prefix, cursor, direction)) {
      cursor.endOf(Unit.WORD, Alter.EXTEND);
      if (cursor.end - cursor.start === this.prefix.length)
        continue;
      /** @const @type {string} */
      const newWord = cursor.text;
      if (this.suggestions_.includes(newWord)) {
        this.log(0, `skip "${newWord}" at`, cursor);
        continue;
      }
      this.log(0, `found "${newWord}" at`, cursor);
      this.suggestions_.push(newWord);
      return true;
    }
    this.log(0, 'No more word in direction', this.direction_);
    return false;
  }

  /**
   * @private
   * @return {boolean} True if suggestions is updated.
   *
   * Move |cursor| to the word starts with |prefix| except for |currentWord|.
   */
  updateSuggetions() {
    switch (this.direction_) {
      case 0:
        return false;
      case 1:
        if (this.tryAddSuggestion(1))
          return true;
        this.direction_ = 0;
        return false;
      case -1:
        if (this.tryAddSuggestion(-1))
          return true;
        this.direction_ = 1;
        this.cursor.collapseTo(this.lastSelectionRange.end);
        return this.updateSuggetions();
    }
    throw new Error(`Unexpected direction ${this.direction_}`);
  }

  /**
   * @param {!TextDocument} document
   * @return {!Session}
   */
  static getOrCreate(document) {
    /** @const @type {?Session} */
    const session = sessionMap.get(document) || null;
    if (session)
      return /** @type {!Session} */ (session);
    const newSession = new Session(document);
    sessionMap.set(document, newSession);
    return newSession;
  }
}

/** @constructor */
suggestions.Session = Session;

Editor.bindKey(TextWindow, 'Ctrl+/', /** @this {!TextWindow} */ function() {
  /** @const @type {!Session} */
  const session = Session.getOrCreate(this.document);
  /** @const @type {string} */
  const response = session.expand(this.selection.range);
  if (response === '')
    return;
  this.status = response;
});

});
