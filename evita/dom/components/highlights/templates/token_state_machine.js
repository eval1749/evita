// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');
goog.require('highlights.HighlightEngine');

goog.scope(function() {

{% if id %}
const HighlightEngine = highlights.HighlightEngine;
const Painter = highlights.Painter;
const Tokenizer = highlights.Tokenizer;
{% endif %}
const TokenStateMachine = highlights.TokenStateMachine;

/** @const @type {number} */
const kMaxAlphabet= {{ max_alphabet }};

/** @const @type {number} */
const kMaxState = {{ max_state }};

/** @const @type {!Uint8Array}
 *   0: All characters expect below
{% for char_codes in alphabet_map %}
{% if loop.index0 != 0 %}
 *   {{ loop.index0 }}: {{ char_codes }}
{% endif %}
{% endfor %}
 */
const kCharCodeToAlphabets = new {{alphabet_type}}Array([
{% for alphabet in char_code_to_alphabet_map %}
  {{ alphabet }},{% if loop.index % 16 == 0 %}{{ '\n' }}{% endif %}
{%- endfor %}
]);

/** @const @type {!Array<string>} */
const kStateToTokenMap = [
{% for state in states %}
    '{{ state.token_type }}', // {{state.index}}:{{state.comment}}
{% endfor %}
];

/** @const @type {!Array<boolean>} */
const kIsAcceptableState = [
{% for state in states %}
{%   if state.is_acceptable %}
  true, // {{state.index}}:{{state.comment}} ACCEPT
{%   else %}
  false, // {{state.index}}:{{state.comment}}
{%   endif %}
{% endfor %}
];

/** @const @type {!Array<!{{state_type}}Array>} */
const kTransitionMap = [
{% for state in states %}
  new {{state_type}}Array({{state.transitions}}), // {{state.index}}:{{state.comment}}
{% endfor %}
];

/**
 * @implements {highlights.TokenStateMachine}
 */
class {{Name}}TokenStateMachine {
  constructor() {
    this.state_ = 0;
  }

  /**
   * @public
   * @param {number} charCode
   * @param {number} state
   * @return {number}
   */
  computeNextState(charCode, state) {
    /** @const @type {number} */
    const alphabet = charCode >= 128 ? 0 : kCharCodeToAlphabets[charCode];
    // DCHECK_LE(alphabet, kMaxAlphabet);
    /** @const @type {number} */
    const newState = kTransitionMap[this.state_][alphabet];
    // DCHECK_LE(newState, kMaxState);
    return newState;
  }

  /** @public @return {number} */
  get state() { return this.state_; }

  /**
   * @public
   * @param {number} state
   * @return {boolean}
   */
  isAcceptable(state) {
    console.assert(state >= 0 && state <= kMaxState, state);
    return kIsAcceptableState[state];
  }

  /**
   * @param {number} newState
   */
  resetTo(newState) {
    console.assert(newState >= 0 && newState <= kMaxState, newState);
    this.state_ = newState;
  }

  /**
   * @public
   * @param {number} state
   * @return {string}
   */
  syntaxOf(state) {
    console.assert(state >= 0 && state <= kMaxState, state);
    return kStateToTokenMap[state];
  }

  /**
   * implements TokenStateMachine.updateState
   * @param {number} charCode
   * @return {number}
   */
  updateState(charCode) {
    /** @const @type {number} */
    const newState = this.computeNextState(charCode, this.state_);
    this.state_ = newState;
    return newState;
  }

  /**
   * @public
   * @return {string}
   */
  toString() {
    return `{{Name}}TokenStateMachine(state: ${this.state_})`;
  }
}

/** @constructor */
highlights.{{Name}}TokenStateMachine = {{Name}}TokenStateMachine;

{% if id %}
/** @const @type {!Set<string>} */
const static{{Name}}Keywords = new Set();

class {{Name}}HighlightEngine extends HighlightEngine {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document, highlights.{{Name}}Painter.create,
          new {{Name}}TokenStateMachine());
  }

  /**
   * @public
   * @param {string} word
   * Adds a keyword at runtime.
   */
  static addKeyword(word) { static{{Name}}Keywords.add(word); }

  /**
   * @param {!TextDocument} document
   * @return {!{{Name}}HighlightEngine}}
   */
  static create(document) { return new {{Name}}HighlightEngine(document); }

  /**
   * @public
   * @return {!Set<string>}
   * For debugging
   */
  static get keywords() { return static{{Name}}Keywords; }
}

/**
 * @param {!TextDocument} document
 * @return {!Tokenizer}
 */
function create{{Name}}Tokenizer(document) {
  /** @const @type {!TokenStateMachine} */
  const stateMachine = new {{Name}}TokenStateMachine();
  /** @const @type {!Painter} */
  const painter = highlights.{{Name}}Painter.create(document);
  return new Tokenizer(document, painter, stateMachine);
}

highlights.create{{Name}}Tokenizer = create{{Name}}Tokenizer;

{% if keywords %}
[
{% for keyword in keywords %}
  '{{keyword}}',
{% endfor %}
].forEach(word => static{{Name}}Keywords.add(word));
{% endif %}

HighlightEngine.registerEngine(
    '{{id}}', {{Name}}HighlightEngine.create, static{{Name}}Keywords);
{% endif %}
});
