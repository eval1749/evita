// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('highlights.base');

goog.scope(function() {

/** @const */
const OrderedSet = base.OrderedSet;

/** @const */
const asStringLiteral = base.asStringLiteral;

/**
 * @interface
 */
const TokenStateMachine = function() {};

/**
 * @param {number} state
 * @return {boolean}
 */
TokenStateMachine.prototype.isAcceptable = function(state) {};

/**
 * @param {number} state
 */
TokenStateMachine.prototype.resetTo = function(state) {};

/**
 * @param {number} state
 * @return {string}
 */
TokenStateMachine.prototype.syntaxOf = function(state) {};

/**
 * @param {number} state
 * @return {number}
 */
TokenStateMachine.prototype.updateState = function(state) {};

class StateRange {
  /**
   * @public
   * @param {!TextDocument} document For debugging.
   * @param {number} start
   * @param {number} end
   * @param {number} state
   * @param {!Token} token
   */
  constructor(document, start, end, state, token) {
    /** @const @type {!TextDocument} */
    this.document_ = document;
    /** @type {number} */
    this.end_ = end;
    /** @type {number} */
    this.state_ = state;
    /** @type {number} */
    this.start_ = start;
    /** @type {!Token} */
    this.token_ = token;
  }

  /** @public @return {number} */
  get end() { return this.end_; }

  /** @public @param {number} newEnd */
  set end(newEnd) {
    console.assert(newEnd >= this.start_, newEnd, this.start_);
    this.end_ = newEnd;
  }

  /** @public @return {number} */
  get length() { return this.end_ - this.start_; }

  /** @public @return {number} */
  get start() { return this.start_; }

  /** @public @param {number} newStart */
  set start(newStart) { this.start_ = newStart; }

  /** @public @return {number} */
  get state() { return this.state_; }

  /** @public @param {number} newState */
  set state(newState) { this.state_ = newState; }

  /** @public @return {string} */
  get syntax() { return this.token_.syntax; }

  /** @public @return {!Token} */
  get token() { return this.token_; }

  /** @public @param {!Token} newToken */
  set token(newToken) { this.token_ = newToken; }

  /**
   * @public
   * @param {number} offset
   */
  reset(offset) {
    this.end_ = offset;
    this.state_ = -1;
    this.start_ = offset;
  }

  /**
   * @public
   * @param {!StateRange} range1
   * @param {!StateRange} range2
   * @return {boolean}
   */
  static less(range1, range2) { return range1.end_ < range2.end_; }

  /**
   * @public
   * @return {string}
   */
  toString() {
    const text = asStringLiteral(this.document_.slice(this.start_, this.end_));
    return `StateRange(${this.start_}, ${this.end_}, ${text}, ` +
        `state:${this.state_}, '${this.token.syntax}')`;
  }
}

class StateRangeMap {
  /**
   * @public
   * @param {!TextDocument} document
   */
  constructor(document) {
    /** @const @type {!TextDocument} */
    this.document_ = document;

    const token = new Token(document, -1, -1, '');
    /** @const @type {!StateRange} */
    this.dummyStateRange_ = new StateRange(document, -1, -1, -1, token);

    /** @const @type {!OrderedSet<!StateRange>} */
    this.ranges_ = new OrderedSet(StateRange.less);
  }

  /** @return {!TextDocument} */
  get document() { return this.document_; }

  /**
   * @public
   * @param {number} start
   * @param {number} end
   * @param {number} state
   * @param {!Token} token
   * @return {!StateRange}
   */
  add(start, end, state, token) {
    console.assert(start <= end, start, end);
    console.assert(start >= 0, start);
    console.assert(end <= this.document_.length, end);
    const newStateRange =
        new StateRange(this.document_, start, end, state, token);
    this.ranges_.add(newStateRange);
    return newStateRange;
  }

  /**
   * @param {number} headCount
   * @param {number} tailCount
   * @param {number} delta
   */
  didChangeTextDocument(headCount, tailCount, delta) {
    if (headCount === 0 && tailCount === 0) {
      this.ranges_.clear();
      return;
    }
    const oldLength = this.document_.length - delta;
    const cleanStart = oldLength - tailCount;
    DVLOG(
        0, 'headCount', headCount, 'oldLength', oldLength, 'delta', delta,
        'cleanStart', cleanStart);
    for (;;) {
      let runner = this.lowerBound(headCount + 1);
      if (runner === null) {
        // There are no nodes after |headCount|.
        return;
      }
      /** @const @type {!StateRange} */
      const range = runner.data;
      /** @const @type {!Token} */
      const token = range.token;
      if (range.start < headCount) {
        // Shrink range contains |headCount|.
        if (token.end === range.end)
          token.end = headCount;
        range.end = headCount;
        continue;
      }
      if (range.start <= cleanStart) {
        // Remove a range between |headCount| to |tailCount|.
        const next = runner.next();
        if (next && next.data.token === token) {
          // We remove ranges with same syntax start from |cleanStart| for
          // comment and string.
          this.removeNode(next);
          continue;
        }
        this.removeNode(runner);
        continue;
      }
      // Relocate ranges.
      while (runner !== null) {
        /** @const @type {!StateRange} */
        const range = runner.data;
        /** @const @type {!Token} */
        const token = range.token;
        if (token.start === range.start)
          token.start += delta;
        if (token.end === range.end)
          token.end += delta;
        range.start += delta;
        range.end += delta;
        runner = runner.next();
      }
      return;
    }
  }

  /**
   * @private
   * @param {number} offset
   * @return {OrderedSetNode<!StateRange>}
   */
  lowerBound(offset) {
    this.dummyStateRange_.reset(offset);
    return this.ranges_.lowerBound(this.dummyStateRange_);
  }

  /**
   * @public
   * @param {!StateRange} range
   */
  remove(range) { this.ranges_.remove(range); }

  /**
   * @public
   * @param {number} start
   * @param {number} end
   */
  removeBetween(start, end) {
    DVLOG(0, start, end);
    for (;;) {
      const node = this.lowerBound(start + 1);
      if (node === null || node.data.start >= end)
        return;
      DVLOG(0, start, end, node.data);
      this.removeNode(node);
    }
  }

  /**
   * @private
   * @param {!OrderedSetNode<!StateRange>} node
   */
  removeNode(node) {
    DVLOG(0, node.data);
    this.ranges_.removeNode(node);
  }

  /**
   * @public
   * @param {number} offset
   * @return {StateRange}
   */
  rangeBefore(offset) {
    if (offset === 0 || this.ranges_.size === 0)
      return null;
    /** @type {OrderedSetNode<!StateRange>} */
    const node = this.lowerBound(offset);
    if (node === null)
      return this.ranges_.maximum;
    const range = node.data;
    if (range.start >= offset)
      return null;
    return range;
  }

  /**
   * @public
   * @param {number} offset
   * @return {StateRange}
   * Returns A |StateRange| ends at |offset|.
   */
  rangeEndsAt(offset) {
    if (offset === 0)
      return null;
    const node = this.lowerBound(offset);
    if (node === null)
      return null;
    const range = node.data;
    if (range.end !== offset)
      return null;
    return range;
  }

  /**
   * @public
   * @param {number} offset
   * @return {StateRange}
   * Returns A |StateRange| contains |offset|.
   */
  rangeStartsAt(offset) {
    /** @type {OrderedSetNode<!StateRange>} */
    const node = this.lowerBound(offset + 1);
    if (node === null)
      return null;
    /** @type {!StateRange} */
    const range = node.data;
    if (range.start !== offset)
      return null;
    return range;
  }

  /**
   * For debugging
   */
  * ranges() { yield * this.ranges_.values(); }
}

class Painter {
  /**
   * @protected
   * @param {!TextDocument} document
   */
  constructor(document) {
    /** @const @type {!TextDocument} */
    this.document_ = document;
  }

  /** @return {!TextDocument} */
  get document() { return this.document_; }

  /**
   * @protected
   * @param {!Token} token
   */
  paintToken(token) { this.paintToken2(token, token.syntax); }

  /**
   * @protected
   * @param {!Token} token
   * @param {string} syntax
   */
  paintToken2(token, syntax) {
    this.document.setSyntax(token.start, token.end, syntax);
  }

  /**
   * @public
   * @param {!Token} token
   */
  paint(token) { this.paintToken(token); }

  /**
   * @protected
   * @param {!Token} token
   * @return {string}
   */
  textOf(token) { return this.document_.slice(token.start, token.end); }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!Painter}
   */
  static create(document) { return new Painter(document); }
}

class Token {
  /**
   * @public
   * @param {!TextDocument} document For debugging.
   * @param {number} start
   * @param {number} end
   * @param {string} syntax
   */
  constructor(document, start, end, syntax) {
    /** @const @type {!TextDocument} */
    this.document_ = document;
    /** @type {number} */
    this.end_ = end;
    /** @type {number} */
    this.start_ = start;
    /** @type {string} */
    this.syntax_ = syntax;
  }

  /** @public @return {number} */
  get end() { return this.end_; }

  /** @public @param {number} newEnd */
  set end(newEnd) {
    console.assert(newEnd >= this.start_, newEnd, this.start_);
    this.end_ = newEnd;
  }

  /** @public @return {number} */
  get length() { return this.end_ - this.start_; }

  /** @public @return {number} */
  get start() { return this.start_; }

  /** @public @param {number} newStart */
  set start(newStart) { this.start_ = newStart; }

  /** @public @return {string} */
  get syntax() { return this.syntax_; }

  /** @public @param {string} newSyntax */
  set syntax(newSyntax) { this.syntax_ = newSyntax; }

  /**
   * @public
   * @return {string}
   */
  toString() {
    const text = asStringLiteral(this.document_.slice(this.start_, this.end_));
    return `Token(${this.start_}, ${this.end_}, ${text}, '${this.syntax}')`;
  }
}

/**
 * Construct |StateRangeMap| by using |TokenStateMachine|.
 */
class Tokenizer {
  /**
   * @public
   * @param {!TextDocument} document
   * @param {!Painter} painter
   * @param {!TokenStateMachine} stateMachine
   */
  constructor(document, painter, stateMachine) {
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
    this.verbose_ = -1;
  }

  /** @return {!TextDocument} */
  get document() { return this.document_; }

  /** @public @return {number} */
  get verbose() { return this.verbose_; }

  /** @public @param {number} level */
  set verbose(level) { this.verbose_ = level; }

  /**
   * @public
   * @param {number} headCount
   * @param {number} tailCount
   * @param {number} delta
   */
  didChangeTextDocument(headCount, tailCount, delta) {
    DVLOG(0, headCount, tailCount, delta);
    this.rangeMap_.didChangeTextDocument(headCount, tailCount, delta);
    const cleanStateRange = this.rangeMap_.rangeBefore(headCount);
    if (cleanStateRange === null) {
      this.scanOffset_ = 0;
      return;
    }
    this.scanOffset_ = cleanStateRange.end;
    const state = cleanStateRange.state;
    this.stateMachine_.resetTo(state);
  }

  /**
   * @public
   */
  didLoadTextDocument() {
    this.scanOffset_ = 0;
    this.rangeMap_.didChangeTextDocument(0, 0, 0);
    this.doColor(this.document_.length);
  }

  /**
   * @public
   * @param {number} hint
   */
  doColor(hint) {
    /** @const @type {number} */
    const scanEnd = Math.min(this.scanOffset_ + hint, this.document_.length);
    this.log(0, 'START', this.scanOffset_, scanEnd);
    if (this.scanOffset_ === scanEnd)
      return;

    /** @type {StateRange} */
    let currentRange = this.rangeMap_.rangeEndsAt(this.scanOffset_);
    this.log(0, 'start', this.scanOffset_, 'currentRange', currentRange);

    /** @type {Token} */
    let token = null;
    this.stateMachine_.resetTo(0);
    if (this.scanOffset_ === 0) {
      // Since pattern doesn't support "^", we treat start of document as
      // following of newline character.
      this.stateMachine_.updateState(Unicode.LF);
    } else if (currentRange && !this.shouldResetState(currentRange.state)) {
      // Since, previous range isn't accepted yet, we should extent it.
      this.stateMachine_.resetTo(currentRange.state);
      token = currentRange.token;
    }

    for (let scanOffset = this.scanOffset_; scanOffset < scanEnd;
         ++scanOffset) {
      /** @const @type {number} */
      const lastState = this.stateMachine_.state;
      /** @const @type {number} */
      const charCode = this.document_.charCodeAt(scanOffset);
      /** @type {number} */
      const state = this.updateState(charCode);
      this.log(0, 'scanOffset', scanOffset, charCode, 'state', state);
      if (state === lastState && currentRange)
        continue;
      if (currentRange)
        this.endStateRange(currentRange, scanOffset);
      if (this.shouldResetState(state))
        this.stateMachine_.resetTo(0);
      currentRange = this.rangeMap_.rangeStartsAt(scanOffset);
      if (currentRange !== null && currentRange.state === state) {
        this.log(0, 'Finish early', currentRange);
        this.scanOffset_ = this.document_.length;
        return;
      }
      if (currentRange)
        this.rangeMap_.remove(currentRange);
      currentRange = this.newStateRange(scanOffset, state, token);
      token = this.stateMachine_.state ? currentRange.token : null;
    }
    this.scanOffset_ = scanEnd;
    this.log(0, 'END', this.scanOffset_, currentRange);

    if (currentRange === null)
      return;
    this.endStateRange(currentRange, scanEnd);
  }

  /**
   * @private
   * @param {!StateRange} range
   * @param {number} end
   */
  endStateRange(range, end) {
    this.log(0, 'endStateRange', range.start, end, 'state', range.state);
    if (range.end < end)
      this.rangeMap_.removeBetween(range.end, end);
    range.end = end;
    range.token.end = end;
    this.paintToken(range.token);
  }

  /**
   * @public
   * @return {boolean}
   */
  isFinished() { return this.scanOffset_ === this.document_.length; }

  /**
   * @private
   * @param {number} level
   * @param {...*} args
   */
  log(level, ...args) {
    if (level > this.verbose_)
      return;
    console.log.apply(console, args);
  }

  /**
   * @private
   * @param {number} scanOffset
   * @param {number} state
   * @param {Token} token
   * @return {!StateRange}
   */
  newStateRange(scanOffset, state, token) {
    const syntax = this.stateMachine_.syntaxOf(state);
    if (token === null) {
      token = new Token(this.document_, scanOffset, scanOffset + 1, syntax);
    } else if (token.syntax === '') {
      token.syntax = syntax;
      token.end = scanOffset + 1;
    } else if (token.syntax === syntax) {
      token.end = scanOffset + 1;
    } else {
      token.end = scanOffset;
      token = new Token(this.document_, scanOffset, scanOffset + 1, syntax);
    }
    const range = this.rangeMap_.add(scanOffset, scanOffset + 1, state, token);
    return range;
  }

  /**
   * @private
   * @param {!Token} token
   */
  paintToken(token) { return this.painter_.paint(token); }

  /**
   * @private
   * @param {number} state
   * @return {boolean}
   */
  shouldResetState(state) { return this.stateMachine_.isAcceptable(state); }

  /**
   * Implements Object.toString
   * @public
   * @return {string}
   */
  toString() {
    return `Tokenizer(document: ${this.document_.name},` +
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

function extractSample(document, start, end, maxChars) {
  if (end - start < maxChars)
    return document.slice(start, end);
  const count = Math.floor(maxChars / 2);
  const headEnd = start + count;
  const tailStart = end - count;
  return [document.slice(start, headEnd), document.slice(tailStart, end)].join(
      ' ... ');
}

/**
 * @implements {Runnable}
 */
class Highlighter extends text.SimpleMutationObserverBase {
  /**
   * @param {!TextDocument} document
   * @param {!function(!TextDocument):!Painter} painterCreator
   * @param {!TokenStateMachine} stateMachine
   */
  constructor(document, painterCreator, stateMachine) {
    super(document);
    /** @const @type {!StateRangeMap} */
    const rangeMap = new StateRangeMap(document);
    /** @const @type {!Painter} */
    const painter = painterCreator(document);
    /** @const @type {!Tokenizer} */
    this.tokenizer_ = new Tokenizer(document, painter, stateMachine);
  }

  /**
   * Implements lexers.Lexer.doColor
   * @public
   */
  detach() { this.stopObserving(); }

  /**
   * Implements text.SimpleMutationObserver.didChangeTextDocument
   * @public
   * @param {number} headCount
   * @param {number} tailCount
   * @param {number} delta
   */
  didChangeTextDocument(headCount, tailCount, delta) {
    this.tokenizer_.didChangeTextDocument(headCount, tailCount, delta);
    this.doColor(100);
  }

  /**
   * @override
   * Implements text.SimpleMutationObserver.didChangeTextDocument
   */
  didLoadTextDocument() { this.tokenizer_.didLoadTextDocument(); }

  /**
   * Implements lexers.Lexer.doColor
   * @public
   * @param {number} hint
   */
  doColor(hint) {
    this.tokenizer_.doColor(hint);
    if (this.isFinished())
      return;
    taskScheduler.schedule(this, 0);
  }

  /**
   * @public
   * For debugging.
   */
  dump() {
    /** @type {Token} */
    let token = null;
    for (const range of this.tokenizer_.ranges()) {
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
          ' ', range.start, range.end, `s${range.state}`, `"${range.syntax}"`,
          asStringLiteral(rangeText));
    }
  }

  /**
   * @private
   * @return {boolean}
   */
  isFinished() { return this.tokenizer_.isFinished(); }

  /**
   * @private
   * Implements Runnable.run
   */
  run() {
    /** @type {number}
     * Number of characters to color during scheduled task.
     * This is an experiment for searching right value.
     */
    const kIncrementalCount = 1000;
    this.doColor(kIncrementalCount);
  }
}

const namespace = highlights.base;
/** @constructor */
namespace.Highlighter = Highlighter;
/** @constructor */
namespace.Painter = Painter;
/** @constructor */
namespace.StateRange = StateRange;
/** @constructor */
namespace.StateRangeMap = StateRangeMap;
/** @constructor */
namespace.Token = Token;
/** @constructor */
namespace.Tokenizer = Tokenizer;
/** @interface */
namespace.TokenStateMachine = TokenStateMachine;

});
