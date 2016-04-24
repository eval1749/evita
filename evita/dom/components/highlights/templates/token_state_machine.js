// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');
goog.require('highlights.HighlightEngine');

goog.scope(function() {

const HighlightEngine = highlights.HighlightEngine;

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

  // TODO(eval1749): We should get rid of |apply()| once we finish testing
  // of scanner generator.
  // For testing of this scanner
  apply(document) {
    const machine = this;
    const ranges = [];

    function endRange(range, offset) {
      range.end = offset;
      range.text = document.slice(range.start, range.end);
      ranges.push(range);
      paint(range);
    }

    let lastPaintOffset = 0;
    function paint(range) {
      const start = range.start;
      const end = range.end;
      const state = range.state;
      const syntax = machine.syntaxOf(state);
      if (syntax === '')
        return;
      const text = document.slice(lastPaintOffset, end)
            .replace(/\\/g, '\\\\')
            .replace(/\x22/g, '\\"')
            .replace(/\n/g, '\\n');
      console.log(`paint[${ranges.length}]`,
                  `s${state} ${lastPaintOffset}, ${end} "${text}":${syntax}`);
      document.setSyntax(lastPaintOffset, end, syntax);
      lastPaintOffset = end;
    }

    function updateState(charCode) {
      const state = machine.updateState(charCode);
      if (state !== 0)
        return state;
      // When |state| is zero, the last state is an acceptable state and
      // no more consumes input. Thus, we need to compute new state with
      // current input.
      return machine.updateState(charCode);
    }

    function vchr(charCode) {
      if (charCode === Unicode.LF) return '\\n';
      if (charCode === Unicode.QUOTATION_MARK) return '\\"';
      if (charCode === Unicode.REVERSE_SOLIDUS) return '\\\\';
      return String.fromCharCode(charCode);
    }

    this.resetTo(0);
    this.updateState(Unicode.LF);
    let range = null;
    document.setSyntax(0, document.length, '');
    for(let offset = 0; offset < document.length; ++offset) {
      const lastState = this.state;
      const charCode = document.charCodeAt(offset);
      const state = updateState(charCode);
      if (state === lastState && range)
        continue;
      console.log('  transit', offset,
                  `s${lastState}-"${vchr(charCode)}"->s${state}`,
                  this.isAcceptable(state) ? 'ACCEPT' : 'continue');
      if (range)
        endRange(range, offset);
      if (this.isAcceptable(state))
        this.resetTo(0);
      range = {start: offset, end: offset + 1, state: state};
    }
    endRange(range, document.length);
    return ranges;
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

{% if keywords %}
[
{% for keyword in keywords %}
  '{{keyword}}',
{% endfor %}
].forEach(word => static{{Name}}Keywords.add(word));
{% endif %}

// Override |{{Name}}Lexer| by |{{Name}}HighlightEngine|.
// TODO(eval1749): Once we get rid of |{{Name}}Lexer|, we should get rid of this
// override.
global['{{Name}}Lexer'] = {{Name}}HighlightEngine;

HighlightEngine.registerEngine(
    '{{id}}', {{Name}}HighlightEngine.create, static{{Name}}Keywords);
{% endif %}
});
