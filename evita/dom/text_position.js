// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Note: When I put |Bracket| into IIFE. Closure compiler can't find
// |Bracket.Type| and  |Bracket.Data|.
var Bracket = {};

/** @enum {number} */
Bracket.Type = {
  NONE: 0,
  ESCAPE: 2,
  LEFT: -1,
  RIGHT: 1,
};

/**
 * @constructor
 * @struct
 * @param {!Bracket.Type} type
 * @param {number} pair
 */
Bracket.Detail = function(type, pair) {
  this.type = type;
  this.pair = pair;
}
/** @type{number} */ Bracket.Detail.prototype.pair;
/** @type{!Bracket.Type} */ Bracket.Detail.prototype.type;

(function() {
  /** @const @type {Object.<number, !Bracket.Detail>} */
  var BRACKET_DATA_OF = {};
  BRACKET_DATA_OF[Unicode.LEFT_PARENTHESIS] = new Bracket.Detail(
      Bracket.Type.LEFT, Unicode.RIGHT_PARENTHESIS);
  BRACKET_DATA_OF[Unicode.RIGHT_PARENTHESIS] = new Bracket.Detail(
      Bracket.Type.RIGHT, Unicode.LEFT_PARENTHESIS);
  BRACKET_DATA_OF[Unicode.LEFT_SQUARE_BRACKET] = new Bracket.Detail(
      Bracket.Type.LEFT, Unicode.RIGHT_SQUARE_BRACKET);
  BRACKET_DATA_OF[Unicode.RIGHT_SQUARE_BRACKET] = new Bracket.Detail(
      Bracket.Type.RIGHT, Unicode.LEFT_SQUARE_BRACKET);
  BRACKET_DATA_OF[Unicode.LEFT_CURLY_BRACKET] = new Bracket.Detail(
      Bracket.Type.LEFT, Unicode.RIGHT_CURLY_BRACKET);
  BRACKET_DATA_OF[Unicode.RIGHT_CURLY_BRACKET] = new Bracket.Detail(
      Bracket.Type.RIGHT, Unicode.LEFT_CURLY_BRACKET);
  BRACKET_DATA_OF[Unicode.REVERSE_SOLIDUS] = new Bracket.Detail(
      Bracket.Type.ESCAPE, 0x00);

  /** @const @type {!Bracket.Detail} */
  var NOT_BRACKET = new Bracket.Detail(Bracket.Type.NONE, 0);

  /** @const @type {number} */
  var MAX_BRACKET_NESTING_LEVEL = 10;

  /**
   * @constructor
   * @struct
   * @param {!Bracket.Detail} data
   * @param {!TextOffset} offset
   */
  function BracketMatchData(data, offset) {
    this.data = data;
    this.offset = offset;
  }
  /** @type{!Bracket.Detail} */ BracketMatchData.prototype.data;
  /** @type{number} */ BracketMatchData.prototype.offset;

  /**
   * @param {!TextPosition} position
   * @return {!Bracket.Detail}
   */
  function bracketDataOf(position) {
    var char_code = position.charCode();
    // TODO(yosi) We should get character syntax from mime type information.
    return BRACKET_DATA_OF[char_code] || NOT_BRACKET;
  }

  /**
   * @param {!TextPosition} position
   * Note: We use syntax coloring information for preventing matching
   * parenthesis among statement, string and comment.
   * See also findRightBracket().
   */
  function moveBackwardBracket(position) {
    /** @type {Array.<BracketMatchData>} */ var bracket_stack = [];
    // reset when we reache at racket.
    /**  @type {number} */ var bracket_char_syntax = -1;
    /** @type {number} */ var start_offset = position.offset;

    while (position.offset) {
      position.move(Unit.CHARACTER, -1);
      // TODO(yosi) We should get character syntax from mime type information.
      var bracket = bracketDataOf(position);
      if (bracket.type == Bracket.Type.NONE)
        continue;
      if (bracket_char_syntax < 0)
        bracket_char_syntax = position.charSyntax();
      else if (position.charSyntax() != bracket_char_syntax)
        continue;

      var current_offset = position.offset;
      position.moveWhile(function() {
        return this.charSyntax() == bracket_char_syntax &&
               bracketDataOf(this).type == Bracket.Type.ESCAPE;
      }, Count.BACKWARD);
      if ((current_offset - position.offset) & 1)
        continue;

      var next_offset = position.offset;
      position.offset = current_offset;

      if (bracket.type == Bracket.Type.LEFT) {
        if (!bracket_stack.length) {
          // We reach at left bracket.
          return;
        }
        var last_bracket = bracket_stack.pop();
        if (last_bracket.data.pair != position.charCode()) {
          // We reach at mismatched left bracket.
          position.move(Unit.CHARACTER);
          return;
        }

        if (!bracket_stack.length) {
          // We reach at matched left bracket.
          return;
        }
        position.offset = next_offset;
      } else if (bracket.type == Bracket.Type.RIGHT) {
        if (!bracket_stack.length) {
          if (position.offset != start_offset - 1) {
            // We found right bracket.
            position.move(Unit.CHARACTER);
            return;
          }
        }
        bracket_stack.push({data: bracket, offset: position.offset});
        position.offset = next_offset;
      }
    }
  }

  /**
   * @param {!TextPosition} position
   * Note: We use syntax coloring information for preventing matching
   * parenthesis among statement, string and comment.
   * See also findLeftBracket().
   */
  function moveForwardBracket(position) {
    /** @type {Array.<BracketMatchData>} */ var bracket_stack = [];
    // reset when we reache at racket.
    /** @type {number} */ var bracket_char_syntax = -1;
    /** @type {number} */ var start_offset = position.offset;

    for (; position.offset < position.document.length;
         position.move(Unit.CHARACTER)) {
      var bracket = bracketDataOf(position);
      if (!bracket)
        continue;
      if (bracket_char_syntax < 0)
        bracket_char_syntax = position.charSyntax();
      else if (position.charSyntax() != bracket_char_syntax)
        continue;

      switch (bracket.type) {
        case Bracket.Type.ESCAPE: {
          var current_offset = position.offset;
          position.moveWhile(function() {
            return bracketDataOf(this).type == Bracket.Type.ESCAPE &&
                   this.charSyntax() == bracket_char_syntax;
          }, Count.FORWARD);
          if (!((position.offset - current_offset) & 1))
            position.move(Unit.CHARACTER, -1);
          break;
        }
        case Bracket.Type.LEFT:
          if (!bracket_stack.length) {
            if (position.offset != start_offset) {
              // We reach left bracket.
              return;
            }
            bracket_stack.push({data: bracket, offset: position.offset});
            break;
          }

          if (bracket_stack.length == MAX_BRACKET_NESTING_LEVEL) {
            // We found too many left bracket.
            return;
          }
          bracket_stack.push({data: bracket, offset: position.offset});
          break;
        case Bracket.Type.RIGHT:
          if (!bracket_stack.length) {
            // We reach right bracket.
            position.move(Unit.CHARACTER);
            return;
          }
          var last_bracket = bracket_stack.pop();
          if (last_bracket.data.pair != position.charCode()) {
            // We reach mismatched right bracket.
            return;
          }

          if (!bracket_stack.length) {
            // We reach matched right bracket.
            position.move(Unit.CHARACTER);
            return;
          }
          break;
      }
    }
    if (bracket_stack.length)
      position.offset = bracket_stack.pop().offset;
  }

  /**
   * @constructor
   * @param {!Document} document
   * @param {!number} offset
   */
  global.TextPosition = function(document, offset) {
    if (offset < 0 || offset > document.length)
      throw new RangeError('Invalid offset ' + offset + ' for ' + document);
    this.document = document;
    this.offset = offset;
  }

  /** * @return {number} */
  global.TextPosition.prototype.charCode = function() {
    return this.document.charCodeAt_(this.offset);
  }

  /** * @return {number} */
  global.TextPosition.prototype.charSyntax = function() {
    return this.document.styleAt(this.offset).charSyntax || 0;
  }

  /**
   * @this {!TextPosition}
   * @param {!Unit} unit
   * @param {number=} opt_count, default is one.
   */
  global.TextPosition.prototype.move = function(unit, opt_count) {
    var count = arguments.length >= 2 ? /** @type{number} */(opt_count) : 1;
    if (unit != Unit.BRACKET)
      this.offset = this.document.computeMotion_(unit, count, this.offset);
    else if (count > 0)
      moveForwardBracket(this);
    else if (count < 0)
      moveBackwardBracket(this);
    return this;
  }

  /**
   * @this {!TextPosition}
   * @param {function() : boolean} callback
   * @param {number=} opt_count, default is one
   * @return {!TextPosition}
   */
  global.TextPosition.prototype.moveWhile = function(callback, opt_count) {
    var count = arguments.length >= 2 ? /** @type{number} */(opt_count) : 1;
    if (count < 0) {
      while (this.offset) {
        --this.offset;
        if (!callback.call(this)) {
          ++this.offset;
          break;
        }
      }
    } else if (count > 0) {
      var end = this.document.length;
      while (this.offset < end) {
        if (!callback.call(this))
          break;
        ++this.offset;
      }
    }
    return this;
  };
})();
