// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

(function() {
  /** @enum{!Symbol} */
  global.Range.Case = {
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

  Object.defineProperties(Range.prototype, {
    length: {get:
        /** @return {number} */
        function() { return this.end - this.start; }
    }
  });

  /**
   * @return {!Range.Case}
   */
  Range.prototype.analyzeCase = function() {
    /** @enum{!Symbol} */
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
    var stringCase = Range.Case.MIXED;
    var state = State.START;
    for (var offset = start; offset < end; ++offset) {
      var charCode = document.charCodeAt_(offset);
      var ucd = Unicode.UCD[charCode];
      var lowerCase = ucd.category == Unicode.Category.Ll ||
                       ucd.category == Unicode.Category.Lt;
      var upperCase = ucd.category == Unicode.Category.Lu;
      switch (state) {
        case State.START:
          if (upperCase) {
            stringCase = Range.Case.CAPITALIZED_WORDS;
            state = State.FIRST_CAP_SECOND;
          } else if (lowerCase) {
            stringCase = Range.Case.LOWER;
            state = State.LOWER;
          }
          break;
        case State.FIRST_CAP_IN_WORD:
          if (upperCase) {
            // We found "FoB".
            return Range.Case.MIXED;
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
            stringCase = Range.Case.CAPITALIZED_TEXT;
            state = State.LOWER;
          }
          break;
        case State.FIRST_CAP_SECOND:
          if (upperCase) {
            // We found "FO"
            stringCase = Range.Case.UPPER;
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
            return Range.Case.MIXED;
          }
          break;
        case State.REST_CAP_IN_WORD:
          if (upperCase) {
            // We found "Foo Bar BaZ"
            return Range.Case.MIXED;
          }
          if (!lowerCase) {
            // We found "Foo Bar+"
            state = State.REST_CAP_NOT_WORD;
          }
          break;
        case State.REST_CAP_NOT_WORD:
          if (lowerCase) {
            // We found "Foo Bar+b"
            return Range.Case.MIXED;
          }
          if (upperCase) {
            // We found "Foo Bar+B"
            state = State.REST_CAP_IN_WORD;
          }
          break;
        case State.UPPER:
          if (lowerCase) {
            // We found "FOo"
            return Range.Case.MIXED;
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
   * @this {!Range}
   * @return {!Range}
   */
  Range.prototype.capitalize = function() {
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
   * @param {number=} opt_count, default is one.
   * @return {!Range}
   */
  Range.prototype.delete = function(unit, opt_count) {
    var count = arguments.length >= 2 ? opt_count : 1;
    var delta = unit == Unit.CHARACTER && this.start != this.end ? 1 : 0;
    if (count < 0)
      this.moveStart(unit, count + delta);
    else if (count > 0)
      this.moveEnd(unit, count - delta);
    this.text = '';
    return this;
  };

  /**
   * Move end position of Range at end of specified unit.
   * @this {!Range}
   * @param {Unit} unit.
   * @param {Alter=} opt_alter, default is Alter.MOVE.
   * @return {!Range}
   */
  Range.prototype.endOf = function(unit, opt_alter) {
    var alter = arguments.length >= 2 ? opt_alter : Alter.MOVE;
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
   * @param {!Editor.RegExp} regexp
   * @return {?Array.<string>}
   */
  Range.prototype.match = function(regexp) {
    var range = this;
    var start = range.start;
    var end = range.end;
    var matches = range.document.match_(regexp, start, end);
    if (!matches)
      return null;
    var strings = matches.map(function(match) {
      return range.document.slice(match.start, match.end);
    });
    range.collapseTo(start);
    range.end = end;
    return strings;
  };

  /**
   * @this {!Range}
   * @param {Unit} unit.
   * @param {number=} opt_count, default is one.
   * @return {!Range}
   */
  Range.prototype.move = function(unit, opt_count) {
    var count = arguments.length >= 2 ? /** @type {number} */(opt_count) : 1;
    var position = count > 0 ? this.end : this.start;
    this.collapseTo(this.document.computeMotion_(unit, count, position));
    return this;
  };

  /**
   * @this {!Range}
   * @param {Unit} unit.
   * @param {number=} opt_count, default is one.
   * @return {!Range}
   */
  Range.prototype.moveEnd = function(unit, opt_count) {
    var count = arguments.length >= 2 ? /** @type {number} */(opt_count) : 1;
    var position = this.document.computeMotion_(unit, count, this.end);
    if (position >= this.start)
      this.end = position;
    else
      this.collapseTo(position);
    return this;
  };

  /**
   * @param {string} charSet
   * @param {number=} opt_count, default is Count.FORWARD
   * @return {!Range}
   */
  Range.prototype.moveEndWhile = function(charSet, opt_count) {
    var count = arguments.length >= 2 ? /** @type{number} */(opt_count) :
        Count.FORWARD;
    var position = this.document.computeWhile_(charSet, count, this.end);
    if (position < this.start)
      this.collapseTo(position);
    else
      this.end = position;
    return this
  };

  /**
   * @this {!Range}
   * @param {Unit} unit.
   * @param {number=} opt_count, default is one.
   * @return {!Range}
   */
  Range.prototype.moveStart = function(unit, opt_count) {
    var count = arguments.length >= 2 ? /** @type {number} */(opt_count) : 1;
    var position = this.document.computeMotion_(unit, count, this.start);
    if (position <= this.end)
      this.start = position;
    else
      this.collapseTo(position);
    return this;
  };

  /**
   * @param {string} charSet
   * @param {number=} opt_count, default is Count.FORWARD
   * @return {!Range}
   */
  Range.prototype.moveStartWhile = function(charSet, opt_count) {
    var count = arguments.length >= 2 ? /** @type{number} */(opt_count) :
        Count.FORWARD;
    var position = this.document.computeWhile_(charSet, count, this.end);
    if (position > this.end)
      this.collapseTo(position);
    else
      this.start = position;
    return this
  };

  /**
   * Move start position of Range at start of specified unit.
   * @param {Unit} unit.
   * @param {Alter=} opt_alter, default is Alter.MOVE.
   * @return {!Range}
   */
  Range.prototype.startOf = function(unit, opt_alter) {
    var alter = arguments.length >= 2 ? opt_alter : Alter.MOVE;
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
   * @return {!Range}
   */
  Range.prototype.toLocaleLowerCase = function() {
    this.text = this.text.toLocaleLowerCase();
    return this;
  };
  
  /**
   * @return {!Range}
   */
  Range.prototype.toLocaleUpperCase = function() {
    this.text = this.text.toLocaleUpperCase();
    return this;
  };
  
  /**
   * @return {!Range}
   */
  Range.prototype.toLowerCase = function() {
    this.text = this.text.toLowerCase();
    return this;
  };
  
  /**
   * @return {!Range}
   */
  Range.prototype.toUpperCase = function() {
    this.text = this.text.toUpperCase();
    return this;
  };
})();
