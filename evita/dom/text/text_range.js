// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

(function() {
  /** @enum{!symbol} */
  global.TextRange.Case = {
    // "This is capitalized."
    CAPITALIZED_TEXT: Symbol('CAPITALIZED_TEXT'),
    // "This Is Capitalized Words."
    CAPITALIZED_WORDS: Symbol('CAPITALIZED_WORDS'),
    // "this is lower."
    LOWER: Symbol('LOWER'),
    // "ThisIsMixed."
    MIXED: Symbol('MIXED'),
    // "THIS IS UPPER."
    UPPER: Symbol('UPPER'),
  };

  Object.defineProperties(TextRange.prototype, {
    length: {get:
        /** @return {number} */
        function() { return this.end - this.start; }
    }
  });

  /**
   * @return {!TextRange.Case}
   */
  TextRange.prototype.analyzeCase = function() {
    /** @enum{!symbol} */
    var State = {
      FIRST_CAP_IN_WORD: Symbol('FIRST_CAP_IN_WORD'),
      FIRST_CAP_NOT_WORD: Symbol('FIRST_CAP_NOT_WORD'),
      FIRST_CAP_SECOND: Symbol('FIRST_CAP_SECOND'),
      LOWER: Symbol('LOWER'),
      REST_CAP_IN_WORD: Symbol('REST_CAP_IN_WORD'),
      REST_CAP_NOT_WORD: Symbol('REST_CAP_NOT_WORD'),
      REST_CAP_REST: Symbol('REST_CAP_REST'),
      START: Symbol('START'),
      UPPER: Symbol('UPPER'),
    };
    var document = this.document;
    var start = this.start;
    var end = this.end;
    var stringCase = TextRange.Case.MIXED;
    var state = State.START;
    for (var offset = start; offset < end; ++offset) {
      var charCode = document.charCodeAt(offset);
      var ucd = Unicode.UCD[charCode];
      var lowerCase = ucd.category == Unicode.Category.Ll ||
                       ucd.category == Unicode.Category.Lt;
      var upperCase = ucd.category == Unicode.Category.Lu;
      switch (state) {
        case State.START:
          if (upperCase) {
            stringCase = TextRange.Case.CAPITALIZED_WORDS;
            state = State.FIRST_CAP_SECOND;
          } else if (lowerCase) {
            stringCase = TextRange.Case.LOWER;
            state = State.LOWER;
          }
          break;
        case State.FIRST_CAP_IN_WORD:
          if (upperCase) {
            // We found "FoB".
            return TextRange.Case.MIXED;
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
            stringCase = TextRange.Case.CAPITALIZED_TEXT;
            state = State.LOWER;
          }
          break;
        case State.FIRST_CAP_SECOND:
          if (upperCase) {
            // We found "FO"
            stringCase = TextRange.Case.UPPER;
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
            return TextRange.Case.MIXED;
          }
          break;
        case State.REST_CAP_IN_WORD:
          if (upperCase) {
            // We found "Foo Bar BaZ"
            return TextRange.Case.MIXED;
          }
          if (!lowerCase) {
            // We found "Foo Bar+"
            state = State.REST_CAP_NOT_WORD;
          }
          break;
        case State.REST_CAP_NOT_WORD:
          if (lowerCase) {
            // We found "Foo Bar+b"
            return TextRange.Case.MIXED;
          }
          if (upperCase) {
            // We found "Foo Bar+B"
            state = State.REST_CAP_IN_WORD;
          }
          break;
        case State.UPPER:
          if (lowerCase) {
            // We found "FOo"
            return TextRange.Case.MIXED;
          }
          break;
        default:
          throw 'Unexpected state ' + state;
      }
    }
    return stringCase;
  };

  /**
   * Capitalize range.
   * @this {!TextRange}
   * @return {!TextRange}
   */
  TextRange.prototype.capitalize = function() {
    var text = this.text;
    for (var i = 0; i < text.length; ++i) {
      var data = Unicode.UCD[text.charCodeAt(i)];
      if (data.category == Unicode.Category.Lu ||
          data.category == Unicode.Category.Ll ||
          data.category == Unicode.Category.Lt) {
        this.text = text.substr(0, i) + text.charAt(i).toLocaleUpperCase() +
                    text.substr(i + 1).toLocaleLowerCase();
        break;
      }
    }
    return this;
  };

  /**
   * Delete
   * @param {Unit} unit
   * @param {number=} count, default is one.
   * @return {!TextRange}
   */
  TextRange.prototype.delete = function(unit, count = 1) {
    var delta = unit == Unit.CHARACTER && this.start != this.end ? 1 : 0;
    if (count < 0)
      this.moveStart(unit, count + delta);
    else if (count > 0)
      this.moveEnd(unit, count - delta);
    this.text = '';
    return this;
  };

  /**
   * Move end position of TextRange at end of specified unit.
   * @this {!TextRange}
   * @param {Unit} unit
   * @param {Alter=} alter, default is Alter.MOVE.
   * @return {!TextRange}
   */
  TextRange.prototype.endOf = function(unit, alter = Alter.MOVE) {
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
  };

  /**
   * @this {!TextRange}
   * @param {!Editor.RegExp} regexp
   * @return {?Array.<string>}
   */
  function rangeMatch(regexp) {
    const document = this.document;
    const matches = document.match_(regexp, this.start, this.end);
    if (!matches)
      return null;
    return matches.map(match => document.slice(match.start, match.end));
  }

  /**
   * @this {!TextRange}
   * @param {!Editor.RegExp} regexp
   */
  function* rangeMatches(regexp) {
    /** @type {number} */
    let runner = this.start;
    while (runner < this.end) {
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
  TextRange.prototype.move = function(unit, count = 1) {
    var position = count > 0 ? this.end : this.start;
    this.collapseTo(this.document.computeMotion_(unit, count, position));
    return this;
  };

  /**
   * @this {!TextRange}
   * @param {Unit} unit
   * @param {number=} count, default is one.
   * @return {!TextRange}
   */
  TextRange.prototype.moveEnd = function(unit, count = 1) {
    var position = this.document.computeMotion_(unit, count, this.end);
    if (position >= this.start)
      this.end = position;
    else
      this.collapseTo(position);
    return this;
  };

  /**
   * @param {string} charSet
   * @param {number=} count, default is Count.FORWARD
   * @return {!TextRange}
   */
  TextRange.prototype.moveEndWhile = function(charSet, count = Count.FORWARD) {
    var position = this.document.computeWhile_(charSet, count, this.end);
    if (position < this.start)
      this.collapseTo(position);
    else
      this.end = position;
    return this
  };

  /**
   * @this {!TextRange}
   * @param {Unit} unit
   * @param {number=} count, default is one.
   * @return {!TextRange}
   */
  TextRange.prototype.moveStart = function(unit, count = 1) {
    var position = this.document.computeMotion_(unit, count, this.start);
    if (position <= this.end)
      this.start = position;
    else
      this.collapseTo(position);
    return this;
  };

  /**
   * @param {string} charSet
   * @param {number=} count, default is Count.FORWARD
   * @return {!TextRange}
   */
  TextRange.prototype.moveStartWhile = function(charSet, count = Count.FORWARD) {
    var position = this.document.computeWhile_(charSet, count, this.end);
    if (position > this.end)
      this.collapseTo(position);
    else
      this.start = position;
    return this
  };

  /**
   * Move start position of TextRange at start of specified unit.
   * @param {Unit} unit
   * @param {Alter=} alter, default is Alter.MOVE.
   * @return {!TextRange}
   */
  TextRange.prototype.startOf = function(unit, alter = Alter.MOVE) {
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
  };

  /**
   * @return {!TextRange}
   */
  TextRange.prototype.toLocaleLowerCase = function() {
    this.text = this.text.toLocaleLowerCase();
    return this;
  };

  /**
   * @return {!TextRange}
   */
  TextRange.prototype.toLocaleUpperCase = function() {
    this.text = this.text.toLocaleUpperCase();
    return this;
  };

  /**
   * @return {!TextRange}
   */
  TextRange.prototype.toLowerCase = function() {
    this.text = this.text.toLowerCase();
    return this;
  };

  /**
   * @return {!TextRange}
   */
  TextRange.prototype.toUpperCase = function() {
    this.text = this.text.toUpperCase();
    return this;
  };

  Object.defineProperties(TextRange.prototype, {
    match: {value: rangeMatch},
    matches: {value: rangeMatches},
  });
})();
