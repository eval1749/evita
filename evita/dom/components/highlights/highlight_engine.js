// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('highlights.HighlightEngine');

goog.require('highlights');

goog.scope(function() {

const asStringLiteral = base.asStringLiteral;

const Painter = highlights.Painter;
const Token = highlights.Token;
const TokenStateMachine = highlights.TokenStateMachine;
const Tokenizer = highlights.Tokenizer;

/** @typedef {function(!TextDocument):!HighlightEngine} */
var HighlightEngineCreator;

/** @const @type {!Map<string, !HighlightEngineCreator>} */
const staticEngineMap = new Map();

/** @const @type {!Map<string, !Set<string>>} */
const staticKeywordsMap = new Map();


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
 *
 * |HighlightEngine| provides basic functionality of syntax highlighting with
 * document mutation. Language specific syntax highlighting is implemented in
 * tokenization state machine and painter.
 */
class HighlightEngine extends text.SimpleMutationObserverBase {
  /**
   * @param {!TextDocument} document
   * @param {!function(!TextDocument):!Painter} painterCreator
   * @param {!TokenStateMachine} stateMachine
   */
  constructor(document, painterCreator, stateMachine) {
    super(document);
    /** @const @type {!Painter} */
    const painter = painterCreator(document);
    /** @const @type {!Tokenizer} */
    this.tokenizer_ = new Tokenizer(document, painter, stateMachine);
  }

  /**
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
   * @public
   * @param {number} hint
   */
  doColor(hint) {
    this.tokenizer_.doColor(hint);
    if (this.isFinished())
      return;
    /**
     * @const @type {number}
     * Since average typing speed is greater than 100ms, we don't need to run
     * so frequently.
     */
    const kIntervalMs = 100;
    taskScheduler.schedule(this, kIntervalMs);
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
          ' ', range.start, range.end, `s${range.state}`,
          asStringLiteral(rangeText));
    }
  }

  /** @public @return {number} */
  get verbose() { return this.tokenizer_.verbose; }

  /** @public @param {number} newVerbose */
  set verbose(newVerbose) { this.tokenizer_.verbose = newVerbose; }

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
    /**
     * @const @type {number}
     * Number of characters to color during scheduled task.
     * This is an experiment for searching right value.
     */
    const kIncrementalCount = 1000;
    this.doColor(kIncrementalCount);
  }

  /**
   * @param {string} name
   * @param {!TextDocument} document
   * @return {!HighlightEngine}
   */
  static createEngine(name, document) {
    const creator = staticEngineMap.get(name) || null;
    if (!creator)
      throw new Error(`No engine for '${name}'`);
    return creator.call(this, document);
  }

  /**
   * @param {string} name
   * @return {!Set<string>}
   */
  static keywordsFor(name) {
    const keywords = staticKeywordsMap.get(name);
    if (!keywords)
      throw new Error(`No keywords for '${name}'`);
    return keywords;
  }

  /**
   * @param {string} name
   * @param {!HighlightEngineCreator} creator
   * @param {!Set<string>} keywords
   */
  static registerEngine(name, creator, keywords) {
    staticEngineMap.set(name, creator);
    staticKeywordsMap.set(name, keywords);
  }
}

/** @constructor */
highlights.HighlightEngine = HighlightEngine;
});
