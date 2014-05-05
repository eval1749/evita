// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.ConfigLexer = (function() {
  /** @const @type {!Map.<number, !Symbol>} */
  var CHARACTERS = (function() {
    var attrs = new Map();
    attrs.set(Unicode.LF, Lexer.WHITESPACE_CHAR);
    attrs.set(Unicode.SPACE, Lexer.WHITESPACE_CHAR);
    attrs.set(Unicode.TAB, Lexer.WHITESPACE_CHAR);
    attrs.set(Unicode.APOSTROPHE, Lexer.STRING1_CHAR);
    attrs.set(Unicode.QUOTATION_MARK, Lexer.STRING2_CHAR);
    return attrs;
  })();

  /** @const @type {!Map.<!Symbol, string>} */
  var STATE_TO_SYNTAX = new Map();

  /**
   * @constructor
   * @extends {Lexer}
   * @param {!Document} document
   */
  function ConfigLexer(document) {
    Lexer.call(this, document, {
      characters: CHARACTERS,
      keywords: [],
      stateToSyntax: STATE_TO_SYNTAX
    });
  }

  /**
   * @this {!ConfigLexer}
   * @param {number} maxOffset
   */
  function nextToken(maxOffset) {
    var lexer = this;
    var document = lexer.range.document;
    while (lexer.scanOffset < maxOffset) {
      var charCode = document.charCodeAt_(lexer.scanOffset);
      if (lexer.state == Lexer.State.ZERO && charCode == Unicode.NUMBER_SIGN) {
        lexer.startToken(Lexer.State.LINE_COMMENT);
        continue;
      }
      this.updateState(charCode);
    }
  }

  ConfigLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: ConfigLexer},
    nextToken: {value: nextToken}
  });

  return ConfigLexer;
})();
