// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('highlights.Tokenizer');

goog.require('base.Logger');
goog.require('highlights');

goog.scope(function() {

const Logger = base.Logger;
const Painter = highlights.Painter;
const StateRange = highlights.StateRange;
const StateRangeMap = highlights.StateRangeMap;
const Token = highlights.Token;
const TokenStateMachine = highlights.TokenStateMachine;
const asStringLiteral = base.asStringLiteral;

function extractSample(document, start, end, maxChars) {
  if (end - start < maxChars)
    return document.slice(start, end);
  const count = Math.floor(maxChars / 2);
  const headEnd = start + count;
  const tailStart = end - count;
  return [document.slice(start, headEnd), document.slice(tailStart, end)].join(
      ' ... ');
}

/*
 * |Tokenizer| populates |StateRangeMap| by using |TokenStateMachine| and
 * asks |Painter| to paint tokens to language specific way.
 */
class Tokenizer extends Logger {
  /**
   * @public
   * @param {!TextDocument} document
   * @param {!Painter} painter
   * @param {!TokenStateMachine} stateMachine
   */
  constructor(document, painter, stateMachine) {
    super();

    /** @const @type {!TextDocument} */
    this.document_ = document;
    /** @const @type {!Painter} */
    this.painter_ = painter;
    /** @const @type {!TokenStateMachine} */
    this.stateMachine_ = stateMachine;
    /** @type {number} */
    this.scanOffset_ = 0;
    /** @const @type {!StateRangeMap} */
    this.rangeMap_ = new StateRangeMap(document);
    /** @type {number} */
    this.verbose_ = 0;
  }

  /** @return {!TextDocument} */
  get document() { return this.document_; }

  /**
   * @public
   * @param {number} headCount
   * @param {number} tailCount
   * @param {number} delta
   */
  didChangeTextDocument(headCount, tailCount, delta) {
    this.log(0, 'didChangeTextDocument', headCount, tailCount, delta);
    this.painter_.didChangeTextDocument(headCount, tailCount, delta);
    this.rangeMap_.didChangeTextDocument(headCount, tailCount, delta);
    if (this.scanOffset_ < headCount) {
      this.log(0, 'change occurred beyond processed area');
      return;
    }
    const cleanStateRange = this.rangeMap_.rangeBefore(headCount);
    if (cleanStateRange === null) {
      this.scanOffset_ = 0;
      this.log(0, 'Restart from start of document');
      return;
    }
    this.scanOffset_ = cleanStateRange.end;
    const state = cleanStateRange.state;
    this.stateMachine_.resetTo(state);
    this.log(0, 'Restart after', cleanStateRange);
  }

  /**
   * @public
   */
  didLoadTextDocument() {
    this.painter_.didLoadTextDocument();
    this.rangeMap_.didChangeTextDocument(0, 0, 0);
    this.scanOffset_ = 0;
  }

  /**
   * @public
   * @param {number} hint
   */
  doColor(hint) {
    /** @const @type {number} */
    const end = this.document_.length;
    /** @const @type {number} */
    const scanEnd = Math.min(this.scanOffset_ + hint, end);
    this.scanOffset_ = this.processRange(this.scanOffset_, scanEnd, 0, end);
  }

  /**
   * @private
   * @param {!StateRange} range
   * @param {number} end
   */
  endRange(range, end) {
    this.log(0, 'endRange', range, end);
    if (range.end < end)
      this.rangeMap_.removeBetween(range.end, end);
    range.end = end;
    Token.setEnd(range.token, end);
  }

  /**
   * @private
   * @param {Token} token
   */
  endToken(token) {
    if (token === null)
      return;
    this.log(0, 'paint', token);
    this.painter_.paint(token);
  }

  /**
   * @public
   * @return {boolean}
   */
  isFinished() { return this.scanOffset_ === this.document_.length; }

  /**
   * @public
   * @param {number} regionStart
   * @param {number} regionEnd
   * @param {number} startOffset
   * @param {number} endOffset
   */
  process(regionStart, regionEnd, startOffset, endOffset) {
    if (this.scanOffset_ >= regionEnd)
      return;
    /** @const @type {number} */
    const scanStart = Math.max(this.scanOffset_, regionStart);
    this.scanOffset_ =
        this.processRange(scanStart, regionEnd, startOffset, endOffset);
  }

  /**
   * @private
   * @param {number} scanStart
   * @param {number} scanEnd
   * @param {number} startOffset
   * @param {number} endOffset
   * @return {number}
   */
  processRange(scanStart, scanEnd, startOffset, endOffset) {
    this.log(0, 'processRange', 'START', scanStart, scanEnd);
    if (scanStart >= scanEnd)
      return scanStart;

    /** @type {StateRange} */
    let range = this.rangeMap_.rangeEndsAt(scanStart);
    this.log(0, 'start', 'range', range);

    /** @type {Token} */
    let token = null;
    this.stateMachine_.resetTo(0);
    if (scanStart === startOffset) {
      // Since pattern doesn't support "^", we treat start of document as
      // following of newline character.
      this.stateMachine_.updateState(Unicode.LF);
    } else if (range && !this.shouldResetState(range.state)) {
      // Since, previous range isn't accepted yet, we should extent it.
      this.stateMachine_.resetTo(range.state);
      token = range.token;
    }

    for (let scanOffset = scanStart; scanOffset < scanEnd; ++scanOffset) {
      /** @const @type {number} */
      const lastState = this.stateMachine_.state;
      /** @const @type {number} */
      const charCode = this.document_.charCodeAt(scanOffset);
      /** @type {number} */
      let state = this.stateMachine_.updateState(charCode);
      this.log(0, 'scanOffset', scanOffset, charCode, 'state', state, token);
      if (state === lastState && range)
        continue;
      if (range)
        this.endRange(range, scanOffset);
      if (state === 0) {
        // |charCode| doesn't belong to current token.
        this.endToken(token);
        token = null;
        state = this.stateMachine_.updateState(charCode);
      }

      /** @const @type {StateRange} */
      const nextRange = this.rangeMap_.rangeStartsAt(scanOffset);
      if (nextRange !== null) {
        if (nextRange.state === state) {
          this.log(0, 'finish early', nextRange);
          this.endToken(token);
          return endOffset;
        }
        this.rangeMap_.remove(nextRange);
      }

      token = this.startOrExtendToken(scanOffset, state, token);
      range = this.startRange(scanOffset, state, token);
      if (!this.shouldResetState(state))
        continue;
      // |charCode| terminates current token.
      this.stateMachine_.resetTo(0);
      this.endToken(token);
      token = null;
    }
    this.log(0, 'processRange', 'END', scanEnd, range);

    if (range === null)
      return scanEnd;
    this.endRange(range, scanEnd);
    this.endToken(range.token);
    return scanEnd;
  }

  /**
   * @public
   * For debugging.
   */
  dump() {
    /** @type {Token} */
    let token = null;
    for (const range of this.ranges()) {
      if (token !== range.token) {
        token = range.token;
        const tokenText =
            extractSample(this.document, token.start, token.end, 40);
        console.log(
            token.start, token.end, `"${token.syntax}"`,
            asStringLiteral(tokenText));
      }
      const rangeText =
          extractSample(this.document, range.start, range.end, 20);
      console.log(
          ' ', range.start, range.end, `s${range.state}`,
          asStringLiteral(rangeText));
    }
  }

  /**
   * @private
   * @param {number} state
   * @return {boolean}
   */
  shouldResetState(state) { return this.stateMachine_.isAcceptable(state); }

  /**
   * @private
   * @param {number} scanOffset
   * @param {number} state
   * @param {Token} token
   * @return {!Token}
   */
  startOrExtendToken(scanOffset, state, token) {
    const syntax = state === 0 ? 'zero' : this.stateMachine_.syntaxOf(state);
    if (token === null)
      return new Token(this.document_, scanOffset, scanOffset + 1, syntax);

    if (token.syntax === '') {
      Token.setSyntax(token, syntax);
      Token.setEnd(token, scanOffset + 1);
      return token;
    }

    if (token.syntax === syntax) {
      Token.setEnd(token, scanOffset + 1);
      return token;
    }

    Token.setEnd(token, scanOffset);
    this.endToken(token);
    return new Token(this.document_, scanOffset, scanOffset + 1, syntax);
  }

  /**
   * @private
   * @param {number} scanOffset
   * @param {number} state
   * @param {!Token} token
   * @return {!StateRange}
   */
  startRange(scanOffset, state, token) {
    return this.rangeMap_.add(scanOffset, scanOffset + 1, state, token);
  }

  /**
   * Implements Object.toString
   * @public
   * @return {string}
   */
  toString() {
    return `Tokenizer(painter: ${this.painter_.constructor.name},` +
        ` document: '${this.document_.name}',` +
        ` scanOffset: ${this.scanOffset_})`;
  }

  /**
   * @private
   * @param {number} charCode
   * @return {number}
   */
  updateState(charCode) {
    const state = this.stateMachine_.updateState(charCode);
    if (state !== 0)
      return state;
    // When |state| is zero, the last state is an acceptable state and
    // no more consumes input. Thus, we need to compute new state with
    // current input.
    return this.stateMachine_.updateState(charCode);
  }

  /**
   * For debugging
   */
  * ranges() { yield * this.rangeMap_.ranges(); }
}

const namespace = highlights;
/** @constructor */
namespace.Tokenizer = Tokenizer;
});