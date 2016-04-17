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

class Token {
  /**
   * @public
   * @param {!TextDocument} document For debugging.
   * @param {number} start
   * @param {number} end
   * @param {number} state
   * @param {string} syntax
   */
  constructor(document, start, end, state, syntax) {
    /** @const @type {!TextDocument} */
    this.document_ = document;
    /** @type {number} */
    this.end_ = end;
    /** @type {number} */
    this.state_ = state;
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
  get start() { return this.start_; }

  /** @public @param {number} newStart */
  set start(newStart) { this.start_ = newStart; }

  /** @public @return {number} */
  get state() { return this.state_; }

  /** @public @param {number} newState */
  set state(newState) { this.state_ = newState; }

  /** @public @return {string} */
  get syntax() { return this.syntax_; }

  /** @public @param {string} newSyntax */
  set syntax(newSyntax) { this.syntax_ = newSyntax; }

  /**
   * @public
   * @param {number} offset
   */
  reset(offset) {
    this.end_ = offset;
    this.state_ = -1;
    this.start_ = offset;
    this.syntax_ = '';
  }

  /**
   * @public
   * @param {!Token} token1
   * @param {!Token} token2
   * @return {boolean}
   */
  static less(token1, token2) { return token1.end_ < token2.end_; }

  /**
   * @public
   * @return {string}
   */
  toString() {
    const text = asStringLiteral(this.document_.slice(this.start_, this.end_));
    return `Token(${this.start_}, ${this.end_}, ${text}, state:${this.state_},` +
        ` '${this.syntax}')`;
  }
}

class TokenMap {
  /**
   * @public
   * @param {!TextDocument} document
   */
  constructor(document) {
    /** @const @type {!TextDocument} */
    this.document_ = document;

    /** @const @type {!Token} */
    this.dummyToken_ = new Token(document, -1, -1, -1, 'dummy');

    /** @const @type {!OrderedSet<!Token>} */
    this.tokens_ = new OrderedSet(Token.less);
  }

  /** @return {!TextDocument} */
  get document() { return this.document_; }

  /**
   * @public
   * @param {number} start
   * @param {number} end
   * @param {number} state
   * @param {string} syntax
   * @return {!Token}
   */
  add(start, end, state, syntax) {
    console.assert(start <= end, start, end);
    console.assert(start >= 0, start);
    console.assert(end <= this.document_.length, end);
    const newToken = new Token(this.document_, start, end, state, syntax);
    this.tokens_.add(newToken);
    return newToken;
  }

  /**
   * @param {number} headCount
   * @param {number} tailCount
   * @param {number} delta
   */
  didChangeTextDocument(headCount, tailCount, delta) {
    if (headCount === 0 && tailCount === 0) {
      this.tokens_.clear();
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
      const token = runner.data;
      if (token.start < headCount) {
        // Shrink token contains |headCount|.
        token.end = headCount;
        continue;
      }
      if (token.start <= cleanStart) {
        // Remove a token between |headCount| to |tailCount|.
        const next = runner.next();
        if (next && next.data.syntax == token.syntax) {
          // We remove tokens with same syntax start from |cleanStart| for
          // comment and string.
          this.removeNode(next);
          continue;
        }
        this.removeNode(runner);
        continue;
      }
      // Relocate tokens.
      while (runner !== null) {
        runner.data.start += delta;
        runner.data.end += delta;
        runner = runner.next();
      }
      return;
    }
  }

  /**
   * @private
   * @param {number} offset
   * @return {OrderedSetNode<!Token>}
   */
  lowerBound(offset) {
    this.dummyToken_.reset(offset);
    return this.tokens_.lowerBound(this.dummyToken_);
  }

  /**
   * @public
   * @param {!Token} token
   */
  remove(token) { this.tokens_.remove(token); }

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
   * @param {!OrderedSetNode<!Token>} node
   */
  removeNode(node) {
    DVLOG(0, node.data);
    this.tokens_.removeNode(node);
  }

  /**
   * @public
   * @param {number} offset
   * @return {Token}
   */
  tokenBefore(offset) {
    if (offset === 0 || this.tokens_.size === 0)
      return null;
    /** @type {OrderedSetNode<!Token>} */
    const node = this.lowerBound(offset);
    if (node === null)
      return this.tokens_.maximum;
    const token = node.data;
    if (token.start >= offset)
      return null;
    return token;
  }

  /**
   * @public
   * @param {number} offset
   * @return {Token}
   * Returns A |Token| ends at |offset|.
   */
  tokenEndsAt(offset) {
    if (offset === 0)
      return null;
    const node = this.lowerBound(offset);
    if (node === null)
      return null;
    const token = node.data;
    if (token.end !== offset)
      return null;
    return token;
  }

  /**
   * @public
   * @param {number} offset
   * @return {Token}
   * Returns A |Token| contains |offset|.
   */
  tokenStartsAt(offset) {
    /** @type {OrderedSetNode<!Token>} */
    const node = this.lowerBound(offset + 1);
    if (node === null)
      return null;
    /** @type {!Token} */
    const token = node.data;
    if (token.start !== offset)
      return null;
    return token;
  }

  /**
   * For debugging
   */
  * tokens() { yield * this.tokens_.values(); }
}

class Painter {
  /**
   * @protected
   * @param {!TextDocument} document
   * @param {!TokenMap} tokenMap
   */
  constructor(document, tokenMap) {
    /** @const @type {!TextDocument} */
    this.document_ = document;
    /** @const @type {!TokenMap} */
    this.tokenMap_ = tokenMap;
  }

  /** @return {!TextDocument} */
  get document() { return this.document_; }

  /** @return {!TokenMap} */
  get tokenMap() { return this.tokenMap_; }

  /**
   * @protected
   * @param {!Token} token
   */
  paintToken(token) {
    this.document.setSyntax(token.start, token.end, token.syntax);
  }

  /**
   * @public
   * @param {!Token} token
   */
  paint(token) { this.paintToken(token); }

  /**
   * @protected
   * @param {!Token} token
   * @return {Token}
   */
  previousTokenOf(token) { return this.tokenMap_.tokenEndsAt(token.start); }

  /**
   * @protected
   * @param {!Token} token
   * @return {string}
   */
  textOf(token) { return this.document_.slice(token.start, token.end); }

  /**
   * @public
   * @param {!TextDocument} document
   * @param {!TokenMap} tokenMap
   * @return {!Painter}
   */
  static create(document, tokenMap) { return new Painter(document, tokenMap); }
}

/**
 * Construct |TokenMap| by using |TokenStateMachine|.
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
    /** @const @type {!TokenMap} */
    this.tokenMap_ = painter.tokenMap;
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
   * @private
   * @param {!Token} token
   */
  paintToken(token) {
    DVLOG(0, token);
    return this.painter_.paint(token);
  }

  /**
   * @public
   * @param {number} headCount
   * @param {number} tailCount
   * @param {number} delta
   */
  didChangeTextDocument(headCount, tailCount, delta) {
    DVLOG(0, headCount, tailCount, delta);
    this.tokenMap_.didChangeTextDocument(headCount, tailCount, delta);
    const cleanToken = this.tokenMap_.tokenBefore(headCount);
    if (cleanToken === null) {
      this.scanOffset_ = 0;
      return;
    }
    this.scanOffset_ = cleanToken.end;
    const state = cleanToken.state;
    this.stateMachine_.resetTo(state);
  }

  /**
   * @public
   */
  didLoadTextDocument() {
    this.scanOffset_ = 0;
    this.tokenMap_.didChangeTextDocument(0, 0, 0);
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

    /** @type {Token} */
    let currentToken = this.tokenMap_.tokenEndsAt(this.scanOffset_);
    this.log(0, 'start', this.scanOffset_, 'currentToken', currentToken);
    if (currentToken) {
      this.stateMachine_.resetTo(currentToken.state);
    } else {
      this.stateMachine_.resetTo(0);
      if (this.scanOffset_ === 0) {
        // Since pattern doesn't support "^", we treat start of document as
        // following of newline character.
        this.stateMachine_.updateState(Unicode.LF);
      }
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
      if (state === lastState && currentToken)
        continue;
      if (currentToken)
        this.endToken(currentToken, scanOffset);
      if (this.stateMachine_.isAcceptable(state))
        this.stateMachine_.resetTo(0);
      currentToken = this.tokenMap_.tokenStartsAt(scanOffset);
      if (currentToken === null) {
        currentToken = this.newToken(scanOffset, state);
        continue;
      }
      if (currentToken.state === state) {
        this.log(0, 'Finish early', currentToken);
        this.scanOffset_ = this.document_.length;
        return;
      }
      this.tokenMap_.remove(currentToken);
      currentToken = this.newToken(scanOffset, state);
    }
    this.scanOffset_ = scanEnd;
    this.log(0, 'END', this.scanOffset_, currentToken);

    if (currentToken === null)
      return;
    this.endToken(currentToken, scanEnd);
  }

  /**
   * @private
   * @param {!Token} token
   * @param {number} end
   */
  endToken(token, end) {
    this.log(0, end, token);
    if (token.end < end)
      this.tokenMap_.removeBetween(token.end, end);
    token.end = end;
    this.paintToken(token);
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
   * @return {!Token}
   */
  newToken(scanOffset, state) {
    const token = this.tokenMap_.add(
        scanOffset, scanOffset + 1, state, this.stateMachine_.syntaxOf(state));
    this.log(0, token);
    return token;
  }

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
  * tokens() { yield * this.tokenMap_.tokens(); }
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
   * @param {!function(!TextDocument, !TokenMap):!Painter} painterCreator
   * @param {!TokenStateMachine} stateMachine
   */
  constructor(document, painterCreator, stateMachine) {
    super(document);
    /** @const @type {!TokenMap} */
    const tokenMap = new TokenMap(document);
    /** @const @type {!Painter} */
    const painter = painterCreator(document, tokenMap)
                    /** @const @type {!Tokenizer} */
                    this.tokenizer_ =
        new Tokenizer(document, painter, stateMachine);
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
    for (const token of this.tokenizer_.tokens()) {
      const text = extractSample(this.document, token.start, token.end, 40);
      console.log(
          token.start, token.end, `s${token.state}`, `"${token.syntax}"`,
          `${asStringLiteral(text)}`);
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
namespace.Token = Token;
/** @constructor */
namespace.Tokenizer = Tokenizer;
/** @constructor */
namespace.TokenMap = TokenMap;
/** @interface */
namespace.TokenStateMachine = TokenStateMachine;

});
