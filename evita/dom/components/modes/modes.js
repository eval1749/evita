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
    document.addEventListener(
        Event.Names.NEWFILE, this.didNewFileTextDocument.bind(document));
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
    /** @type {!string} */
    const newModeId = Mode.chooseMode(document);
    /** @type {string} */
    const currentModeName = document.mode ? document.mode.name : '';
    if (newModeId === currentModeName)
      return;
    Editor.messageBox(
        null, `Change mode to ${newModeId}`, MessageBox.ICONINFORMATION);
    document.mode = Mode.create(newModeId);
  }

  /**
   * @this {!TextDocument}
   */
  didNewFileTextDocument() {
    this.mode = Mode.create(Mode.chooseModeByFileName(this.fileName));
    TextDocument.applyTemplate(this);
  }

  /** @param {!TextDocument} document */
  didRemoveTextDocument(document) { document.mode = null; }
}

TextDocument.addObserver(new Observer());

});
