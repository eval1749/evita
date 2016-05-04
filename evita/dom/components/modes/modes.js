// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('modes');

goog.scope(function() {

const Mode = modes.Mode;

class Observer extends SimpleTextDocumentSetObserver {
  constructor() { super(); }

  /** @param {!TextDocument} document */
  didAddTextDocument(document) {
    document.addEventListener(
        Event.Names.LOAD, this.didLoadTextDocument.bind(document));
  }

  /**
   * @this {!TextDocument}
   *
   * Updates document mode by mode property in contents or file name.
   */
  didLoadTextDocument() {
    /** @type {!TextDocument} */
    const document = this;
    document.parseFileProperties();
    /** @type {!Mode} */
    const newMode = Mode.chooseMode(document);
    /** @type {string} */
    const currentModeName = document.mode ? document.mode.name : '';
    if (newMode.name === currentModeName)
      return;
    Editor.messageBox(
        null, `Change mode to ${newMode.name}`, MessageBox.ICONINFORMATION);
    document.mode = /** @type {Mode} */ (newMode);
  }

  /** @param {!TextDocument} document */
  didRemoveTextDocument(document) { document.mode = null; }
}

TextDocument.addObserver(new Observer());

});
