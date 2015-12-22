// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.Mode = (function() {
  class Mode {
    /**
     * @param {string} name
     * @param {!Function} lexerConstructor
     */
    constructor(name, lexerConstructor) {
      /** @type {!Map} */
      this.keymap_ = new Map();
      /** @type {Lexer} */
      this.lexer_ = null;
      /** @type {!Function} */
      this.lexerConstructor_ = lexerConstructor;
      /** @type {string} */
      this.name_ = name;
    }

    /**
     * @this {!Mode}
     * @param {!Document} document
     */
    attach(document) {
      if (this.document_)
        throw new Error(this + ' is already attached to ' + this.document_);
      this.document_ = document;
      this.lexer_ = new this.lexerConstructor_(document);
    }

    /**
     * @this {!Mode}
     */
    detach() {
      if (!this.document_)
        throw new Error(this + ' is already attached to ' + this.document_);
      this.lexer_.detach();
    }

    /**
     * @this {!Mode}
     * @param {!Document} document
     * @param {number} hint
     */
    doColor(document, hint) {
      if (!this.document_)
        throw new Error(this + ' is detached.');
      this.lexer_.doColor(hint);
    }

    get document() { return this.document_; }
    get keymap() { return this.keymap_; }
    get name() { return this.name_; }
  }

  return Mode;
})();
