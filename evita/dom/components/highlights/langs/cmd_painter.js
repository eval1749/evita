// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');

goog.scope(function() {

const Painter = highlights.Painter;
const Token = highlights.Token;

/** @const @type {!Set<string>} */
const staticCmdKeywords = highlights.HighlightEngine.keywordsFor('cmd');

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isAlphabet(charCode) {
  if (charCode >= Unicode.LATIN_SMALL_LETTER_A &&
      charCode >= Unicode.LATIN_SMALL_LETTER_Z) {
    return true;
  }
  if (charCode >= Unicode.LATIN_CAPITAL_LETTER_A &&
      charCode >= Unicode.LATIN_CAPITAL_LETTER_Z) {
    return true;
  }
  return false;
}

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isWhitespace(charCode) {
  return charCode <= Unicode.SPACE;
}

class CmdPainter extends Painter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document); }

  /**
   * @override
   * @param {!Token} token
   */
  paint(token) {
    /** @type {number} */
    let runner = token.start;
    while (runner < token.end && isWhitespace(this.document.charCodeAt(runner)))
      ++runner;
    if (runner > token.start)
      this.setSyntax(token.start, runner, 'normal');
    if (runner === token.end)
      return;
    if (this.document.charCodeAt(runner) === Unicode.COLON) {
      ++runner;
      if (runner === token.end)
        return this.setSyntax(runner - 1, token.end, 'normal');
      if (isWhitespace(this.document.charCodeAt(runner)))
        return this.setSyntax(runner - 1, token.end, 'comment');
      return this.setSyntax(runner - 1, token.end, 'label');
    }
    /** @const @type {number} */
    const nameStart = runner;
    while (runner < token.end && isAlphabet(this.document.charCodeAt(runner)))
      ++runner;
    if (nameStart === runner)
      return this.setSyntax(runner, token.end, 'normal');
    /** @const @type {string} */
    const name = this.document.slice(nameStart, runner);
    if (staticCmdKeywords.has(name))
      this.setSyntax(nameStart, runner, 'keyword');
    else
      this.setSyntax(nameStart, runner, 'identifier');
    if (runner === token.end)
      return;
    if (name === 'rem') {
      this.setSyntax(runner, token.end, 'comment');
      return;
    }
    this.setSyntax(runner, token.end, 'normal');
  }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!CmdPainter}
   */
  static create(document) { return new CmdPainter(document); }
}

// Export |CmdPainter| for testing.
/** @constructor */
highlights.CmdPainter = CmdPainter;
});
