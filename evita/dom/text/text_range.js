// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('unicode');

goog.scope(function() {

/** @enum{string} */
const State = {
  FIRST_CAP_IN_WORD: 'FIRST_CAP_IN_WORD',
  FIRST_CAP_NOT_WORD: 'FIRST_CAP_NOT_WORD',
  FIRST_CAP_SECOND: 'FIRST_CAP_SECOND',
  LOWER: 'LOWER',
  REST_CAP_IN_WORD: 'REST_CAP_IN_WORD',
  REST_CAP_NOT_WORD: 'REST_CAP_NOT_WORD',
  REST_CAP_REST: 'REST_CAP_REST',
  START: 'START',
  UPPER: 'UPPER',
};

/**
 * @this {!TextRange}
 * @return {number}
 */
function getLength() {
  return this.end - this.start;
}

/**
 * @this {!TextRange}
 * @return {!CaseAnalysisResult}
 */
function analyzeCase() {
  /** @const @type {!TextDocument} */
  const document = this.document;
  /** @const @type {number} */
  const start = this.start;
  /** @const @type {number} */
  const end = this.end;
  /** @type {CaseAnalysisResult} */
  let stringCase = CaseAnalysisResult.MIXED;
  /** @type {State} */
  let state = State.START;
  for (let offset = start; offset < end; ++offset) {
    const charCode = document.charCodeAt(offset);
    /** @const @type {boolean} */
    const lowerCase = unicode.isLowerCase(charCode);
    /** @const @type {boolean} */
    const upperCase =
        unicode.isUpperCase(charCode) || unicode.isTitleCase(charCode);
    switch (state) {
      case State.START:
        if (upperCase) {
          stringCase = CaseAnalysisResult.CAPITALIZED_WORDS;
          state = State.FIRST_CAP_SECOND;
        } else if (lowerCase) {
          stringCase = CaseAnalysisResult.LOWER;
          state = State.LOWER;
        }
        break;
      case State.FIRST_CAP_IN_WORD:
        if (upperCase) {
          // We found "FoB".
          return CaseAnalysisResult.MIXED;
        } else if (lowerCase) {
          // We found "Foo".
        } else {
          // We found "Foo+".
          state = State.FIRST_CAP_NOT_WORD;
        }
        break;
      case State.FIRST_CAP_NOT_WORD:
        if (upperCase) {
          // We found "Foo B".
          state = State.REST_CAP_IN_WORD;
        } else if (lowerCase) {
          // We found "Foo b"
          stringCase = CaseAnalysisResult.CAPITALIZED_TEXT;
          state = State.LOWER;
        }
        break;
      case State.FIRST_CAP_SECOND:
        if (upperCase) {
          // We found "FO"
          stringCase = CaseAnalysisResult.UPPER;
          state = State.UPPER;
        } else if (lowerCase) {
          // We found "Fo"
          state = State.FIRST_CAP_IN_WORD;
        } else {
          // We see "F+"
          state = State.FIRST_CAP_NOT_WORD;
        }
        break;
      case State.LOWER:
        if (upperCase) {
          // We found "foB"
          return CaseAnalysisResult.MIXED;
        }
        break;
      case State.REST_CAP_IN_WORD:
        if (upperCase) {
          // We found "Foo Bar BaZ"
          return CaseAnalysisResult.MIXED;
        }
        if (!lowerCase) {
          // We found "Foo Bar+"
          state = State.REST_CAP_NOT_WORD;
        }
        break;
      case State.REST_CAP_NOT_WORD:
        if (lowerCase) {
          // We found "Foo Bar+b"
          return CaseAnalysisResult.MIXED;
        }
        if (upperCase) {
          // We found "Foo Bar+B"
          state = State.REST_CAP_IN_WORD;
        }
        break;
      case State.UPPER:
        if (lowerCase) {
          // We found "FOo"
          return CaseAnalysisResult.MIXED;
        }
        break;
      default:
        throw 'Unexpected state ' + state;
    }
  }
  return stringCase;
}

/**
 * Capitalize range.
 * @this {!TextRange}
 * @return {!TextRange}
 */
function capitalize() {
  /** @const @type {string} */
  const text = this.text;
  for (/** @type {number} */ let i = 0; i < text.length; ++i) {
    if (unicode.isLetter(text.charCodeAt(i))) {
      this.text = text.substr(0, i) + text.charAt(i).toLocaleUpperCase() +
          text.substr(i + 1).toLocaleLowerCase();
      break;
    }
  }
  return this;
}

/**
 * Delete
 * @this {!TextRange}
 * @param {Unit} unit
 * @param {number=} count, default is one.
 * @return {!TextRange}
 */
function deleteRange(unit, count = 1) {
  /** @const @type {number} */
  const delta = unit == Unit.CHARACTER && this.start != this.end ? 1 : 0;
  if (count < 0)
    this.moveStart(unit, count + delta);
  else if (count > 0)
    this.moveEnd(unit, count - delta);
  this.text = '';
  return this;
}

/**
 * Move end position of TextRange at end of specified unit.
 * @this {!TextRange}
 * @param {Unit} unit
 * @param {Alter=} alter, default is Alter.MOVE.
 * @return {!TextRange}
 */
function endOf(unit, alter = Alter.MOVE) {
  /** @const @type {number} */
  this.end = this.document.computeEndOf_(unit, this.end);
  switch (alter) {
    case Alter.EXTEND:
      break;
    case Alter.MOVE:
      this.start = this.end;
      break;
    default:
      throw TypeError('Invalid alter: ' + alter);
  }
  return this;
}

/**
 * @this {!TextRange}
 * @param {!Editor.RegExp} regexp
 * @return {?Array.<string>}
 */
function rangeMatch(regexp) {
  /** @const @type {!TextDocument} */
  const document = this.document;
  /** @const @type {?Array<RegExpMatch>} */
  const matches = document.match_(regexp, this.start, this.end);
  if (!matches)
    return null;
  return matches.map(match => document.slice(match.start, match.end));
}

/**
 * @this {!TextRange}
 * @param {!Editor.RegExp} regexp
 * @return {!Generator<!Array<RegExpMatch>>}
 */
function * rangeMatches(regexp) {
  /** @type {number} */
  let runner = this.start;
  while (runner < this.end) {
    /** @const @type {?Array<RegExpMatch>} */
    const matches = this.document.match_(regexp, runner, this.end);
    if (!matches)
      return;
    runner = matches[0].end;
    yield matches;
  }
}

/**
 * @this {!TextRange}
 * @param {Unit} unit
 * @param {number=} count, default is one.
 * @return {!TextRange}
 */
function move(unit, count = 1) {
  /** @const @type {number} */
  const position = count > 0 ? this.end : this.start;
  this.collapseTo(this.document.computeMotion_(unit, count, position));
  return this;
}

/**
 * @this {!TextRange}
 * @param {Unit} unit
 * @param {number=} count, default is one.
 * @return {!TextRange}
 */
function moveEnd(unit, count = 1) {
  /** @const @type {number} */
  const position = this.document.computeMotion_(unit, count, this.end);
  if (position >= this.start)
    this.end = position;
  else
    this.collapseTo(position);
  return this;
}

/**
 * @this {!TextRange}
 * @param {string} charSet
 * @param {number=} count, default is Count.FORWARD
 * @return {!TextRange}
 */
function moveEndWhile(charSet, count = Count.FORWARD) {
  /** @const @type {number} */
  const position = this.document.computeWhile_(charSet, count, this.end);
  if (position < this.start)
    this.collapseTo(position);
  else
    this.end = position;
  return this
}

/**
 * @this {!TextRange}
 * @param {Unit} unit
 * @param {number=} count, default is one.
 * @return {!TextRange}
 */
function moveStart(unit, count = 1) {
  /** @const @type {number} */
  const position = this.document.computeMotion_(unit, count, this.start);
  if (position <= this.end)
    this.start = position;
  else
    this.collapseTo(position);
  return this;
}

/**
 * @this {!TextRange}
 * @param {string} charSet
 * @param {number=} count, default is Count.FORWARD
 * @return {!TextRange}
 */
function moveStartWhile(charSet, count = Count.FORWARD) {
  /** @const @type {number} */
  const position = this.document.computeWhile_(charSet, count, this.end);
  if (position > this.end)
    this.collapseTo(position);
  else
    this.start = position;
  return this
}

/**
 * Move start position of TextRange at start of specified unit.
 * @this {!TextRange}
 * @param {Unit} unit
 * @param {Alter=} alter, default is Alter.MOVE.
 * @return {!TextRange}
 */
function startOf(unit, alter = Alter.MOVE) {
  /** @const @type {number} */
  this.start = this.document.computeStartOf_(unit, this.start);
  switch (alter) {
    case Alter.EXTEND:
      break;
    case Alter.MOVE:
      this.end = this.start;
      break;
    default:
      throw TypeError('Invalid alter: ' + alter);
  }
  return this;
}

/**
 * @this {!TextRange}
 * @return {!TextRange}
 */
function toLocaleLowerCase() {
  this.text = this.text.toLocaleLowerCase();
  return this;
}

/**
 * @this {!TextRange}
 * @return {!TextRange}
 */
function toLocaleUpperCase() {
  this.text = this.text.toLocaleUpperCase();
  return this;
}

/**
 * @this {!TextRange}
 * @return {!TextRange}
 */
function toLowerCase() {
  this.text = this.text.toLowerCase();
  return this;
}

/**
 * @this {!TextRange}
 * @return {!TextRange}
 */
function toUpperCase() {
  this.text = this.text.toUpperCase();
  return this;
}

Object.defineProperties(TextRange.prototype, {
  analyzeCase: {value: analyzeCase},
  capitalize: {value: capitalize},
  delete: {value: deleteRange},
  endOf: {value: endOf},
  length: {get: getLength},
  match: {value: rangeMatch},
  matches: {value: rangeMatches},
  move: {value: move},
  moveEnd: {value: moveEnd},
  moveEndWhile: {value: moveEndWhile},
  moveStart: {value: moveStart},
  moveStartWhile: {value: moveStartWhile},
  startOf: {value: startOf},
  toLocaleLowerCase: {value: toLocaleLowerCase},
  toLocaleUpperCase: {value: toLocaleUpperCase},
  toLowerCase: {value: toLowerCase},
  toUpperCase: {value: toUpperCase},
});
});
