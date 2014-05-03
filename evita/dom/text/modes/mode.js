// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
* @constructor
* @param {string} name
* @param {!function()} lexerConstructor
*/
global.Mode = (function() {
  function Mode(name, lexerConstructor) {
    this.keymap_ = new Map();
    this.lexerConstructor_ = lexerConstructor;
    this.name_ = name;
  }
  return Mode;
})();

Mode.prototype = Object.defineProperties(Mode.prototype, {
  doColor: {value:
    /**
     * @this {!Mode}
     * @param {!Document} document
     * @param {number} hint
     * @return {number}
     */
    function(document, hint) {
      if (!this.lexer_)
        this.lexer_ = new this.lexerConstructor_(document);
      return this.lexer_.doColor(hint);
    }
  },
  keymap: { get: function() { return this.keymap_; } },
  keymap_: {writable: true},
  lexer_: {value: null, writable: true},
  lexerConstructor_: {writable: true},
  name: { get: function() { return this.name_; } },
  name_: {writable: true},
});
