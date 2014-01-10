// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

Document.prototype.close = function() {
  var document = this;
  if (!document.modified) {
    document.forceClose();
    return;
  }
  Editor.messageBox(null,
                    localizeText(IDS_ASK_SAVE, {name: document.name}),
                    localizeText(IDS_APP_TITLE),
                    MessageBox.ICONWARNING | MessageBox.YESNOCANCEL)
    .then(function(response_code) {
      switch (response_code) {
        case DialogIemId.NO:
          document.forceClose();
          break;
        case DialogIemId.NO:
          Editor.getFilenameForSave(null, document.filename)
            .then(function(filename) {
              document.save(filename).then(function() {
                document.forceClose();
              });
            });
          break;
      }
    });
};

Document.prototype.forceClose = function() {
  this.listWindows().forEach(function(window) {
    window.destroy();
  });
  Document.remove(this);
};

/**
 * @return {Array.<TextWindow>}
 */
Document.prototype.listWindows = function() {
  var document = this;
  var windows = [];
  EditorWindow.list.forEach(function(editorWindow) {
    editorWindow.children.forEach(function(window) {
      if ((window instanceof TextWindow) && window.document == document)
        windows.push(window);
    });
  });
  return windows;
};

/**
 * @param {string} A filename to load from.
 * @return {Document} A Document object contains contents of file.
 */
Document.load = function(filename) {
  var absolute_filename = FilePath.fullPath(filename);
  var canonical_filename = absolute_filename.toLocaleLowerCase();
  var present = Document.list.find(function(document) {
    return document.filename.toLocaleLowerCase() == canonical_filename;
  });
  if (present)
    return present;
  var document = new Document(FilePath.basename(filename));
  document.load_(absolute_filename);
  return document;
};
