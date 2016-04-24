// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('highlights');

goog.require('base.Logger');

goog.scope(function() {

const Logger = base.Logger;
const OrderedSet = base.OrderedSet;
const OrderedSetNode = base.OrderedSetNode;
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

/*
 * |StateRange| represents tokenization state machine state in |TextDocument|
 * in range.
 */
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

  /** @public @return {boolean} */
  isValid() {
    return this.start_ < this.end_ && this.start_ >= this.token_.start &&
        this.end_ <= this.token.end_;
  }

  /**
   * @public
   * @param {number} offset
   * This function is used only for initializing search key for
   * |OrderedSet<StateRange>|.
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
   * This function is a used for ordering in |OrderedSet<StateRange>|.
   */
  static less(range1, range2) { return range1.end_ < range2.end_; }

  /**
   * @public
   * @return {string}
   */
  toString() {
    const text = asStringLiteral(this.document_.slice(this.start_, this.end_));
    return `StateRange(${this.start_}, ${this.end_}, ${text}, ` +
        `state:${this.state_}, ${this.token}')`;
  }

  /**
   * @public
   * @param {!StateRange} range
   * @param {number} amount
   */
  static shift(range, amount) {
    const newStart = range.start_ + amount;
    console.assert(newStart >= range.token_.start, range, 'amount', amount);
    /** @const @type {number} */
    const newEnd = range.end_ + amount;
    console.assert(newEnd <= range.token_.end, range, 'amount', amount);
    range.start_ = newStart;
    range.end_ = newEnd;
  }
}

/*
 * Updates |StateRange| for |TextDocument| for document mutations.
 */
class StateRangeMap extends Logger {
  /**
   * @public
   * @param {!TextDocument} document
   */
  constructor(document) {
    super();

    /** @const @type {!TextDocument} */
    this.document_ = document;

    /** @const @type {!Token} */
    const dummyToken = new Token(document, -2, -1, '');

    /** @const @type {!StateRange} */
    this.dummyStateRange_ = new StateRange(document, -1, -1, -1, dummyToken);

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
    console.assert(
        end <= this.document_.length, end, 'max', this.document_.length);
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
    this.log(
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
      if (range.start > cleanStart)
        return this.relocateRanges(runner, delta);

      /** @const @type {!Token} */
      const token = range.token;

      /** @const @type {OrderedSetNode<!StateRange>} */
      const next = runner.next();
      if (next && next.data.token === token) {
        // We remove ranges in same token.
        this.removeNode(next);
        continue;
      }

      if (range.start < headCount) {
        // Shrink |range| since it crosses clean and dirty boundary:
        // ....dirty....
        //   >---<
        if (token.end === range.end)
          Token.setEnd(token, headCount);
        range.end = headCount;
        continue;
      }
      // Remove a range after clean.
      //  ...dirty...
      //      >----<
      this.removeNode(runner);
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
   * @private
   * @param {!OrderedSetNode<!StateRange>} start
   * @param {number} delta
   */
  relocateRanges(start, delta) {
    /** @type {Token} */
    let lastToken = null;
    for (let runner = start; runner; runner = runner.next()) {
      /** @const @type {!Token} */
      const token = runner.data.token;
      if (token === lastToken)
        continue;
      Token.shift(token, delta);
      lastToken = token;
    }
    for (let runner = start; runner; runner = runner.next()) {
      /** @const @type {!StateRange} */
      const range = runner.data;
      StateRange.shift(range, delta);
      console.assert(range.isValid(), range);
    }
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
    this.log(0, 'removeBetween', start, end);
    for (;;) {
      const node = this.lowerBound(start + 1);
      if (node === null || node.data.start >= end)
        return;
      this.removeNode(node);
    }
  }

  /**
   * @private
   * @param {!OrderedSetNode<!StateRange>} node
   */
  removeNode(node) {
    this.log(0, 'removeNode', node.data);
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

  /**
   * @override
   * @return {string}
   */
  toString() { return `StateRangeMap(size=${this.ranges_.size})`; }
}

/*
 * |Token| is a text range in |TextDocument| as an unit of painting with
 * syntax highlighting.
 */
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
    console.assert(start < end, this);
  }

  /** @public @return {number} */
  get end() { return this.end_; }

  /** @public @return {number} */
  get length() { return this.end_ - this.start_; }

  /** @public @return {number} */
  get start() { return this.start_; }

  /** @public @return {string} */
  get syntax() { return this.syntax_; }

  /**
   * @public
   * @return {string}
   */
  toString() {
    const text = asStringLiteral(this.document_.slice(this.start_, this.end_));
    return `Token(${this.start_}, ${this.end_}, ${text}, '${this.syntax}')`;
  }

  /**
   * @public
   * @param {!Token} token
   * @param {number} newEnd
   */
  static setEnd(token, newEnd) {
    console.assert(token.start_ < newEnd, 'newEnd', newEnd, token);
    token.end_ = newEnd;
  }

  /**
   * @public
   * @param {!Token} token
   * @param {string} newSyntax
   */
  static setSyntax(token, newSyntax) { token.syntax_ = newSyntax; }

  /**
   * @public
   * @param {!Token} token
   * @param {number} amount
   */
  static shift(token, amount) {
    /** @const @type {number} */
    const newStart = token.start_ + amount;
    console.assert(
        newStart >= 0 && newStart < token.document_.length, token, 'amount',
        amount, 'max', token.document_.length);
    /** @const @type {number} */
    const newEnd = token.end_ + amount;
    console.assert(
        newEnd >= 0 && newEnd <= token.document_.length, token, 'amount',
        amount, 'max', token.document_.length);
    token.start_ = newStart;
    token.end_ = newEnd;
  }
}

const namespace = highlights;
/** @constructor */
namespace.StateRange = StateRange;
/** @constructor */
namespace.StateRangeMap = StateRangeMap;
/** @constructor */
namespace.Token = Token;
/** @interface */
namespace.TokenStateMachine = TokenStateMachine;
});
