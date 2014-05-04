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
  attach: {value:
    /**
     * @this {!Mode}
     * @param {!Document} document
     */
    function(document) {
      if (this.document_)
        throw new Error(this + ' is already attached to ' + this.document_);
      this.document_ = document;
      this.lexer_ = new this.lexerConstructor_(document);
    }
  },
  detach: {value:
    /**
     * @this {!Mode}
     */
    function() {
      if (!this.document_)
        throw new Error(this + ' is already attached to ' + this.document_);
      this.lexer_.detach();
    }
  },
  document: {get: function() { return this.document_; }},
  document_: {value: null, writable: true},
  doColor: {value:
    /**
     * @this {!Mode}
     * @param {!Document} document
     * @param {number} hint
     * @return {number}
     */
    function(document, hint) {
      if (!this.document_)
        throw new Error(this + ' is detached.');
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
