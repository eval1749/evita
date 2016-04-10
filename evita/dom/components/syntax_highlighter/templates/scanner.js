// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

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

/** @const @type {!Array<number>} */
const kStateToTokenMap = [
{% for state in states %}
    '{{ state.token_type }}', // {{ state.index }}
{% endfor %}
];

/** @const @type {!Array<!{{state_type}}Array>} */
const kTransitionMap = [
{% for state in states %}
  new {{state_type}}Array({{state.transitions}}), // {{loop.index0}}:{{state.comment}}
{% endfor %}
];

/**
 * @implements {lexers.Scanner}
 */
class {{Name}}Scanner {
  constructor() {
    this.state_ = 0;
  }

  // TODO(eval1749): We should get rid of |apply()| once we finish testing
  // of scanner generator.
  // For testing of this scanner
  apply(document) {
    this.resetTo(0);
    let lastOffset = 0;
    let lastState = 0;
    let lastSyntax = '';
    let number_of_tokens = 0
    document.setSyntax(0, document.length, '');
    for(let offset = 0; offset < document.length; ++offset) {
      const charCode = document.charCodeAt(offset);
      const state = this.updateState(charCode);
      if (lastState == state)
        continue
      lastState = state;
      const syntax = this.syntaxFor(state);
      if (lastSyntax == syntax)
        continue
      if (number_of_tokens < 200) {
          console.log(
            `s${lastState}-${kCharCodeToAlphabets[charCode]}->s${this.state_}`,
            lastSyntax, lastOffset, offset);
      }
      document.setSyntax(lastOffset, offset, lastSyntax);
      ++number_of_tokens
      if (lastSyntax == 'identifier')
        console.log(`identifier "${document.slice(lastOffset, offset)}"`);
      lastOffset = offset
      lastSyntax = syntax
    }
    return number_of_tokens;
  }

  /**
   * @param {number} newState
   */
  resetTo(newState) {
    console.assert(newState <= kMaxState, newState);
    this.state_ = newState;
  }

  /**
   * @public
   * @param {number} state
   * @return {string}
   */
  syntaxFor(state) {
    return kStateToTokenMap[state];
  }

  /**
   * implements lexers.Scanner.updateState
   * @param {number} charCode
   * @return {number}
   */
  updateState(charCode) {
    /** @const @type {number} */
    const alphabet = charCode >= 128 ? 0 : kCharCodeToAlphabets[charCode];
    console.assert(alphabet <= kMaxAlphabet, alphabet);
    const newState = kTransitionMap[this.state_][alphabet];
    console.assert(newState <= kMaxState, newState);
    this.state_ = newState;
    return newState;
  }
}

lexers.{{Name}}Scanner = {{Name}}Scanner;
});
