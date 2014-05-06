// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.ConfigLexer = (function() {
  /** @const @type {!Map.<number, !Symbol>} */
  var CHARACTERS = (function() {
    var map = new Map();
    map.set(Unicode.LF, Lexer.WHITESPACE_CHAR);
    map.set(Unicode.SPACE, Lexer.WHITESPACE_CHAR);
    map.set(Unicode.TAB, Lexer.WHITESPACE_CHAR);
    map.set(Unicode.APOSTROPHE, Lexer.STRING1_CHAR);
    map.set(Unicode.QUOTATION_MARK, Lexer.STRING2_CHAR);
    return map;
  })();

  /** @const @type {!Map.<string, string>} */
  var KEYWORDS = Lexer.createKeywords([]);

  /**
   * @constructor
   * @extends {Lexer}
   * @param {!Document} document
   */
  function ConfigLexer(document) {
    Lexer.call(this, document, {
      characters: CHARACTERS,
      keywords: KEYWORDS,
    });
  }

  /**
   * @this {!ConfigLexer}
   * @param {number} charCode
   */
  function feedCharacter(charCode) {
    if (this.state == Lexer.State.ZERO && charCode == Unicode.NUMBER_SIGN) {
      this.startToken(Lexer.State.LINE_COMMENT);
      return;
    }
    this.updateState(charCode);
  }

  ConfigLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: ConfigLexer},
    feedCharacter: {value: feedCharacter}
  });

  return ConfigLexer;
})();
