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

/**
 * @param {!TextDocument} document
 * @param {number} start
 * @param {number} end
 * @param {number} maxChars
 * @return {string}
 *
 * Extracts characters from |start| to |end|, exclusive, from |document|. If
 * number of characters is greater than |maxChars|, middle of characters are
 * replaced with "...".
 */
function extractSample(document, start, end, maxChars) {
  if (end - start < maxChars)
    return document.slice(start, end);
  /** @const @type {number} */
  const count = Math.floor(maxChars / 2);
  /** @const @type {number} */
  const headEnd = start + count;
  /** @const @type {number} */
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
    /** @type {number} */
    this.documentLength_ = document.length;
    /** @const @type {!Painter} */
    this.painter_ = painter;
    /** @type {boolean} */
    this.shouldPaintSkippedToken_ = false;
    /** @const @type {!TokenStateMachine} */
    this.stateMachine_ = stateMachine;
    /** @type {number} */
    this.scanOffset_ = 0;
    /** @const @type {!StateRangeMap} */
    this.rangeMap_ = new StateRangeMap(document);
  }

  /** @return {!TextDocument} */
  get document() { return this.document_; }

  /** @return {number} */
  get maxOffset() {
    return Math.min(this.documentLength_, this.document.length);
  }

  /**
   * @public
   * @return {number}
   * For |HighlightEngine| printer.
   */
  get scanOffset() { return this.scanOffset_; }

  /**
   * @public
   * @param {boolean} value
   */
  set shouldPaintSkippedToken(value) { this.shouldPaintSkippedToken_ = value; }

  /**
   * @private
   * @param {!StateRange} range A |StateRange| to check.
   * @param {number} state  An expected state for |range|.
   *
   * We can use existing |range| if
   *   - |state| equals to |range.start|, and
   *   - |range| is a start of token.
   */
  canUseRange(range, state) {
    if (range.state !== state) {
      this.log(0, `canUseRange: state mismatched s${state}`, range);
      return false;
    }
    if (range.token.start !== range.start) {
      this.log(0, 'canUseRange: range is not start of token', range);
      return false;
    }
    this.log(0, 'canUseRange: REUSE', range);
    return true;
  }

  /**
   * @public
   * @param {number} headCount
   * @param {number} tailCount
   * @param {number} delta
   */
  didChangeTextDocument(headCount, tailCount, delta) {
    this.documentLength_ = this.document.length;
    this.log(0, 'didChangeTextDocument:', headCount, tailCount, delta);
    this.painter_.didChangeTextDocument(headCount, tailCount, delta);
    this.rangeMap_.didChangeTextDocument(headCount, tailCount, delta);
    if (this.scanOffset_ < headCount) {
      this.log(0, 'change occurred beyond processed area', this.scanOffset_);
      return;
    }
    /** @const @type {?StateRange} */
    const cleanStateRange = this.rangeMap_.rangeBefore(headCount);
    if (cleanStateRange === null) {
      this.scanOffset_ = 0;
      this.log(0, 'Restart from start of document');
      return;
    }
    this.scanOffset_ = cleanStateRange.end;
    /** @const @type {number} */
    const state = cleanStateRange.state;
    this.stateMachine_.resetTo(state);
    this.log(0, 'Restart after', cleanStateRange);
    if (this.verbose === 0)
      return;
    this.dump();
  }

  /** @override */
  didChangeVerbose() {
    this.painter_.verbose = this.verbose;
    this.rangeMap_.verbose = this.verbose;
  }

  /**
   * @public
   */
  didLoadTextDocument() {
    this.documentLength_ = this.document.length;
    this.painter_.didLoadTextDocument();
    this.rangeMap_.didChangeTextDocument(0, 0, 0);
    this.scanOffset_ = 0;
  }

  /**
   * @private
   * @param {!Token} token
   */
  didSkipToken(token) {
    if (!this.shouldPaintSkippedToken_)
      return;
    this.paintToken(token);
  }

  /**
   * @public
   * @param {number} hint
   */
  doColor(hint) {
    /** @const @type {number} */
    const end = this.maxOffset;
    /** @const @type {number} */
    const scanEnd = Math.min(this.scanOffset_ + hint, end);
    this.scanOffset_ = this.processRange(this.scanOffset_, scanEnd, 0, end);
  }

  /**
   * @public
   * For debugging.
   */
  dump() {
    /** @type {?Token} */
    let token = null;
    for (const range of this.ranges()) {
      if (token !== range.token) {
        token = range.token;
        /** @const @type {string} */
        const tokenText =
            extractSample(this.document, token.start, token.end, 40);
        console.log(
            token.start, token.end, `"${token.syntax}"`,
            asStringLiteral(tokenText));
      }
      /** @const @type {string} */
      const rangeText =
          extractSample(this.document, range.start, range.end, 20);
      console.log(
          ' ', range.start, range.end, `s${range.state}`,
          asStringLiteral(rangeText));
    }
  }

  /**
   * @private
   * @param {!StateRange} range
   * @param {number} end
   */
  endRange(range, end) {
    this.log(0, 'endRange:', range, end);
    if (range.end < end)
      this.rangeMap_.removeBetween(range.end, end);
    range.end = end;
    Token.setEnd(range.token, end);
  }

  /**
   * @private
   * @param {?Token} token
   */
  endToken(token) {
    if (token === null)
      return;
    this.paintToken(token);
  }

  /**
   * @public
   * @return {boolean}
   */
  isFinished() { return this.scanOffset_ === this.maxOffset; }

  /**
   * @private
   * @param {number} scanOffset
   * @param {number} charCode
   * @param {number} state
   * @param {?Token} token
   */
  logLoopStart(scanOffset, charCode, state, token) {
    if (this.verbose <= 0)
      return;
    this.log(
        0, 'LOOP', scanOffset, asStringLiteral(String.fromCharCode(charCode)),
        `s${state}`, token);
  }

  /**
   * @private
   * @param {!Token} token
   */
  paintToken(token) {
    this.log(0, 'paint:', token);
    this.painter_.paint(token);
  }

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
    this.log(0, 'START:', scanStart, 'to', scanEnd);
    if (scanStart >= scanEnd)
      return scanStart;

    /** @type {?StateRange} */
    let lastRange = this.rangeMap_.rangeEndsAt(scanStart);
    this.log(0, 'start', 'lastRange:', lastRange);
    if (lastRange && lastRange.token.syntax === '') {
      /** @const @type {number} */
      const state = lastRange.state;
      /** @const @type {string} */
      const syntax = this.stateMachine_.syntaxOf(state);
      Token.setSyntax(lastRange.token, syntax);
    }

    /** @type {?Token} */
    let token = null;
    this.stateMachine_.resetTo(0);
    if (scanStart === startOffset) {
      // Since pattern doesn't support "^", we treat start of document as
      // following of newline character.
      this.stateMachine_.updateState(Unicode.LF);
    } else if (lastRange && !this.shouldResetState(lastRange.state)) {
      // Since, previous lastRange isn't accepted yet, we should extent it.
      this.stateMachine_.resetTo(lastRange.state);
      token = lastRange.token;
    }

    for (let scanOffset = scanStart; scanOffset < scanEnd; ++scanOffset) {
      /** @const @type {number} */
      const lastState = this.stateMachine_.state;
      /** @const @type {number} */
      const charCode = this.document_.charCodeAt(scanOffset);
      /** @type {number} */
      let state = this.stateMachine_.updateState(charCode);
      this.logLoopStart(scanOffset, charCode, state, token);
      if (state === lastState && lastRange)
        continue;
      if (lastRange)
        this.endRange(lastRange, scanOffset);
      if (state === 0) {
        // |charCode| doesn't belong to current token.
        this.endToken(token);
        token = null;
        state = this.stateMachine_.updateState(charCode);
      }

      /** @const @type {?StateRange} */
      const currentRange = this.rangeMap_.rangeStartsAt(scanOffset);
      if (currentRange && this.canUseRange(currentRange, state)) {
        // We've already processed |currentRange|, so we can skip it.
        lastRange = this.skipExistingRanges(currentRange, endOffset);
        if (lastRange.end >= scanEnd) {
          this.log(0, 'END skip beyond scanEnd', scanEnd, lastRange);
          this.didSkipToken(lastRange.token);
          return lastRange.end;
        }
        this.log(0, 'skip after:', lastRange);
        token = lastRange.token;
        scanOffset = lastRange.end - 1;
        this.stateMachine_.resetTo(lastRange.state);
        continue;
      }

      if (currentRange)
        this.rangeMap_.remove(currentRange);

      token = this.startOrExtendToken(scanOffset, state, token);
      lastRange = this.startRange(scanOffset, state, token);
      if (!this.shouldResetState(state))
        continue;
      // |charCode| terminates current token.
      this.stateMachine_.resetTo(0);
      this.endToken(token);
      token = null;
    }
    this.log(0, 'END:', scanEnd, lastRange);
    if (lastRange === null)
      return scanEnd;
    this.endRange(lastRange, scanEnd);
    this.endToken(lastRange.token);
    return scanEnd;
  }

  /**
   * @private
   * @param {number} state
   * @return {boolean}
   */
  shouldResetState(state) { return this.stateMachine_.isAcceptable(state); }

  /**
   * @private
   * @param {!StateRange} startRange
   * @param {number} endOffset
   * @return {!StateRange}
   */
  skipExistingRanges(startRange, endOffset) {
    /** @type {!StateRange} */
    let lastRange = startRange;
    while (lastRange.token.end < endOffset) {
      /** @type {?StateRange} */
      const range = this.rangeMap_.rangeStartsAt(lastRange.token.end);
      if (!range)
        break;
      console.assert(lastRange.token !== range.token, lastRange, range);
      this.didSkipToken(lastRange.token);
      lastRange = range;
    }
    /** @type {?StateRange} */
    const endRange = this.rangeMap_.rangeEndsAt(lastRange.token.end);
    console.assert(endRange, lastRange);
    return /** @type {!StateRange} */ (endRange);
  }

  /**
   * @private
   * @param {number} scanOffset
   * @param {number} state
   * @param {?Token} token
   * @return {!Token}
   */
  startOrExtendToken(scanOffset, state, token) {
    /** @const @type {string} */
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
    /** @const @type {number} */
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
   * @return {!Generator<!StateRange>}
   */
  * ranges() { yield * this.rangeMap_.ranges(); }
}

const namespace = highlights;
/** @constructor */
namespace.Tokenizer = Tokenizer;
});
