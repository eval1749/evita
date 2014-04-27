// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.ConfigLexer = (function() {
  /** @enum{number} */
  var State = {
    START: 0, // State.START must be zero.

    COMMENT: 1,
    STRING1: 2,
    STRING1_ESCAPE: 3,
    STRING2: 4,
    STRING2_ESCAPE: 5,
  };

  /**
   * @constructor
   * @extends Lexer
   * @param {!Document} document
   */
  function ConfigLexer(document) {
    Lexer.call(this, document);
  }

  /**
   * @this {!ConfigLexer}
   * @param {number} maxCount
   */
  function doColor(maxCount) {
    if (!this.range)
      throw new Error("Can't use disconnected lexer.");

    this.adjustScanOffset(maxCount);
    var range = this.range;
    var document = range.document;
    var maxOffset = document.length;
    while (this.scanOffset < maxOffset) {
      var syntax = nextToken(this);
      if (!syntax)
        break;
      if (!range.collapsed)
        range.setSyntax(syntax);
    }

    return this.count;
  }

  /**
   * @param {!ConfigLexer} lexer
   * @return {?string}
   */
  function nextToken(lexer) {
    if (!lexer.count)
      return null;
    var document = lexer.range.document;
    var maxOffset = document.length;
    if (lexer.scanOffset == maxOffset)
      return null;
    while (lexer.count) {
      --lexer.count;
      if (lexer.scanOffset == maxOffset)
        break;
      var charCode = document.charCodeAt_(lexer.scanOffset);
      ++lexer.scanOffset;
      switch (lexer.state) {
        case State.COMMENT:
          if (charCode == Unicode.LF)
            return lexer.finishToken();
          break;
        case State.START:
          switch (charCode) {
            case Unicode.APOSTROPHE:
              lexer.startToken(State.STRING1, 'string_literal');
              break;
            case Unicode.NUMBER_SIGN:
              lexer.startToken(State.COMMENT, 'comment');
              break;
            case Unicode.QUOTATION_MARK:
              lexer.startToken(State.STRING2, 'string_literal');
              break;
          }
          break;
        case State.STRING1:
          if (charCode == Unicode.APOSTROPHE)
            return lexer.finishToken();
          if (charCode == Unicode.REVERSE_SOLIDUS)
            lexer.state = State.STRING1_ESCAPE;
          break;
        case State.STRING1_ESCAPE:
          lexer.state = State.STRING1;
          break;
        case State.STRING2:
          if (charCode == Unicode.QUOTATION_MARK)
            return lexer.finishToken();
          if (charCode == Unicode.REVERSE_SOLIDUS)
            lexer.state = State.STRING2_ESCAPE;
          break;
        case State.STRING2_ESCAPE:
          lexer.state = State.STRING2;
          break;
        default:
          throw new Error('Invalid state ' + lexer.state);
      }
    }
    lexer.range.end = lexer.scanOffset;
    return lexer.syntax;
  }

  ConfigLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: ConfigLexer},
    doColor: {value: doColor}
  });

  return ConfigLexer;
})();
