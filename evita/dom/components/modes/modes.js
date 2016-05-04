// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('modes');

goog.scope(function() {

const Mode = modes.Mode;

/** @param {!TextDocument} document */
function attachModeIfNeeded(document) {
  document.parseFileProperties();
  /** @type {!string} */
  const newModeId = Mode.chooseMode(document);
  /** @const @type {?Mode} */
  const currentMode = Mode.modeOf(document);
  if (currentMode && currentMode.id === newModeId)
    return;
  if (currentMode) {
    Editor.messageBox(
        null, `Change mode to ${newModeId}`, MessageBox.ICONINFORMATION);
  }
  Mode.attach(document, newModeId);
}

class Observer extends SimpleTextDocumentSetObserver {
  constructor() { super(); }

  /** @param {!TextDocument} document */
  didAddTextDocument(document) {
    document.addEventListener(
        Event.Names.LOAD, this.didLoadTextDocument.bind(document));
    document.addEventListener(
        Event.Names.NEWFILE, this.didNewFileTextDocument.bind(document));
    attachModeIfNeeded(document);
  }

  /**
   * @this {!TextDocument}
   *
   * Updates document mode by mode property in contents or file name.
   */
  didLoadTextDocument() { attachModeIfNeeded(this); }

  /**
   * @this {!TextDocument}
   */
  didNewFileTextDocument() {
    attachModeIfNeeded(this);
    TextDocument.applyTemplate(this);
  }

  /** @param {!TextDocument} document */
  didRemoveTextDocument(document) { document.mode = null; }
}

TextDocument.list.forEach(document => attachModeIfNeeded);

TextDocument.addObserver(new Observer());

});
