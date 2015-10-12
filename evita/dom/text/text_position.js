// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
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
    var charCode = position.charCode();
    // TODO(yosi) We should get character syntax from mime type information.
    return Bracket.DATA[charCode] || NOT_BRACKET;
  }

  /**
   * @param {string} syntax1
   * @param {string} syntax2
   * @return {boolean}
   */
  function syntaxEquals(syntax1, syntax2) {
    if (syntax1 === syntax2)
      return true;
    return syntax1 === 'normal' || syntax2 == 'normal';
  }

  /**
   * @param {!TextPosition} position
   * Note: We use syntax coloring information for preventing matching
   * parenthesis among statement, string and comment.
   * See also |moveForwardBracket()|.
   */
  function moveBackwardBracket(position) {
    /** @type {Array.<BracketMatchData>} */ var bracketStack = [];
    // reset when we reached at bracket.
    /**  @type {string} */ var bracketCharSyntax = '?';
    /** @type {number} */ var startOffset = position.offset;

    while (position.offset) {
      position.move(Unit.CHARACTER, -1);
      // TODO(yosi) We should get character syntax from mime type information.
      var bracket = bracketDataOf(position);
      if (bracket.type == Bracket.Type.NONE)
        continue;
      if (bracketCharSyntax == '?')
        bracketCharSyntax = position.charSyntax();
      else if (!syntaxEquals(position.charSyntax(), bracketCharSyntax))
        continue;

      var currentOffset = position.offset;
      position.moveWhile(function() {
        return syntaxEquals(this.charSyntax(), bracketCharSyntax) &&
               bracketDataOf(this).type == Bracket.Type.ESCAPE;
      }, Count.BACKWARD);
      if ((currentOffset - position.offset) & 1)
        continue;

      var nextOffset = position.offset;
      position.offset = currentOffset;

      if (bracket.type == Bracket.Type.LEFT) {
        if (!bracketStack.length) {
          // We reach at left bracket.
          return;
        }
        var lastBracket = bracketStack.pop();
        if (lastBracket.data.pair != position.charCode()) {
          // We reach at mismatched left bracket.
          break;
        }

        if (!bracketStack.length) {
          // We reach at matched left bracket.
          return;
        }
        position.offset = nextOffset;
      } else if (bracket.type == Bracket.Type.RIGHT) {
        if (!bracketStack.length) {
          if (position.offset != startOffset - 1) {
            // We found right bracket.
            position.move(Unit.CHARACTER);
            return;
          }
        }
        bracketStack.push({data: bracket, offset: position.offset});
        position.offset = nextOffset;
      }
    }
    position.offset = startOffset;
  }

  /**
   * @param {!TextPosition} position
   * Note: We use syntax coloring information for preventing matching
   * parenthesis among statement, string and comment.
   * See also |moveBackwardBracket()|.
   */
  function moveForwardBracket(position) {
    /** @type {Array.<BracketMatchData>} */ var bracketStack = [];
    // reset when we reached at racket.
    /** @type {string} */ var bracketCharSyntax = '?';
    /** @type {number} */ var startOffset = position.offset;

    for (; position.offset < position.document.length;
         position.move(Unit.CHARACTER)) {
      var bracket = bracketDataOf(position);
      if (bracket.type == Bracket.Type.NONE)
        continue;
      if (bracketCharSyntax == '?')
        bracketCharSyntax = position.charSyntax();
      else if (!syntaxEquals(position.charSyntax(), bracketCharSyntax))
        continue;
      switch (bracket.type) {
        case Bracket.Type.ESCAPE: {
          var currentOffset = position.offset;
          position.moveWhile(function() {
            return bracketDataOf(this).type == Bracket.Type.ESCAPE &&
                   syntaxEquals(this.charSyntax(), bracketCharSyntax);
          }, Count.FORWARD);
          if (!((position.offset - currentOffset) & 1))
            position.move(Unit.CHARACTER, -1);
          break;
        }
        case Bracket.Type.LEFT:
          if (!bracketStack.length) {
            if (position.offset != startOffset) {
              // We reach left bracket.
              return;
            }
            bracketStack.push({data: bracket, offset: position.offset});
            break;
          }

          if (bracketStack.length == MAX_BRACKET_NESTING_LEVEL) {
            // We found too many left bracket.
            return;
          }
          bracketStack.push({data: bracket, offset: position.offset});
          break;
        case Bracket.Type.RIGHT:
          if (!bracketStack.length) {
            // We reach right bracket.
            position.move(Unit.CHARACTER);
            return;
          }
          var lastBracket = bracketStack.pop();
          if (lastBracket.data.pair != position.charCode()) {
            // We reach mismatched right bracket.
            return;
          }

          if (!bracketStack.length) {
            // We reach matched right bracket.
            position.move(Unit.CHARACTER);
            return;
          }
          break;
      }
    }
    if (bracketStack.length)
      position.offset = bracketStack.pop().offset;
    else
      position.offset = startOffset;
  }

  /**
   * @constructor
   * @param {!Document} document
   * @param {!number} offset
   */
  global.TextPosition = (function() {
    function TextPosition(document, offset) {
      if (offset < 0 || offset > document.length)
        throw new RangeError('Invalid offset ' + offset + ' for ' + document);
      this.document = document;
      this.offset = offset;
    }
    return TextPosition;
  })();

  /** * @return {number} */
  global.TextPosition.prototype.charCode = function() {
    return this.document.charCodeAt_(this.offset);
  }

  /** * @return {string} */
  global.TextPosition.prototype.charSyntax = function() {
    return this.document.syntaxAt(this.offset);
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
      while (count && this.offset) {
        --this.offset;
        if (!callback.call(this)) {
          ++this.offset;
          break;
        }
        ++count;
      }
    } else if (count > 0) {
      var end = this.document.length;
      while (count && this.offset < end) {
        if (!callback.call(this))
          break;
        ++this.offset;
        --count;
      }
    }
    return this;
  };
})();
