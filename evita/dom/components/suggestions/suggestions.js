// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Note: Session should be in file scope. Although, when we use
// IIFE, immediately-invoked function expression, Closure compiler can't find
// |Session| type.

goog.provide('suggestions');

goog.scope(function() {

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
  if (text === '' || !direction)
    return false;

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
  } else if (direction < 0) {
    do {
      range.move(Unit.WORD, -1);
      if (range.start + textLength > document.length)
        break;
      if (matchAt(text, document, range.start))
        return true;
    } while (range.start);
    range.collapseTo(0);
  }
  return false;
}

class Session {
  /**
   * @public
   * @param {!TextDocument} document
   */
  constructor(document) {
    /** @const @type {!TextRange} */
    this.cursor = new TextRange(document);
    /** @type {number} */
    this.direction = 0;
    /** @type {TextRange} */
    this.lastSelectionRange = null;
    /** @type {number} */
    this.lastSelectionStart = 0;
    /** @type {string} */
    this.prefix = '';
  }

  /**
   * @public
   * @param {!TextSelection} selection
   */
  expand(selection) {
    /** @const @type {!TextRange} */
    const range = selection.range;
    if (range.start !== range.end)
      return;
    this.startOrContinue(selection);

    /** @type {string} */
    let currentWord = range.text;
    while (this.findCandidate(currentWord)) {
      const newWord = this.cursor.text;
      if (currentWord !== newWord) {
        range.text = newWord;
        range.collapseTo(range.end);
        this.lastSelectionStart = range.start;
        return;
      }
    }

    // Nothing changed.
    range.collapseTo(range.end);
    selection.window.status = currentWord ?
        `No more words start with "${this.prefix}", ` +
            `other than "${currentWord}"` :
        `No word starts with "${this.prefix}"`;
  }

  /**
   * @private
   * @param {string} currentWord
   * @return {boolean}
   *
   * Move |cursor| to the word starts with |prefix| except for |currentWord|.
   */
  findCandidate(currentWord) {
    /** @type {!TextRange} */
    const cursor = this.cursor;
    /** @type {number} */
    const originEnd = cursor.end;
    /** @type {number} */
    const originStart = cursor.start;
    // Try current direction and opposite direction to find a candidate.
    for (/** @type {number} */ let count = 0; count < 2; ++count) {
      if (findWordStartsWith(this.prefix, cursor, this.direction)) {
        cursor.endOf(Unit.WORD, Alter.EXTEND);
        if (cursor.end === originEnd || cursor.start === originStart)
          return false;
        if (cursor.end - cursor.start === this.prefix.length)
          continue;
        if (cursor.text === currentWord)
          continue;
        return true;
      }

      // Continue find a candidate in opposite direction.
      this.direction = -this.direction;
      if (this.direction > 0)
        cursor.collapseTo(this.lastSelectionRange.end);
      else
        cursor.collapseTo(this.lastSelectionRange.start);
    }
    return false;
  }

  /**
   * @param {!TextSelection} selection
   * @return {boolean} False if we should start new session.
   * TODO(eval1749): Check this command and last command
   */
  prepare(selection) {
    if (this.prefix === '')
      return false;
    if (this.lastSelectionRange !== selection.range) {
      // Someone invokes session in different window.
      return false;
    }
    if (this.lastSelectionStart !== selection.range.start) {
      // Selection is moved.
      return false;
    }
    // selection is at end of expanded word.
    selection.range.moveStart(Unit.WORD, -1);
    if (!selection.range.text.startsWith(this.prefix)) {
      selection.range.collapseTo(selection.range.end);
      return false;
    }
    return true;
  }

  /**
   * @private
   * @param {!TextSelection} selection
   */
  startOrContinue(selection) {
    if (this.prepare(selection))
      return;
    /** @const @type {!TextRange} */
    const range = selection.range;
    this.cursor.collapseTo(range.start);
    this.cursor.moveStart(Unit.WORD, -1);
    this.direction = -1;
    this.prefix = this.cursor.text;
    this.lastSelectionRange = range;
    this.lastSelectionStart = range.start;
    range.start = this.cursor.start;
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
  session.expand(this.selection);
});

});
