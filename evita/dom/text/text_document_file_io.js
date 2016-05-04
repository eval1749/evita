// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {
/**
 * @param {string} absoluteFileName
 * @return {TextDocument}
 */
function findFile(absoluteFileName) {
  const canonicalFileName = absoluteFileName.toLocaleLowerCase();
  return /** @type {TextDocument} */ (
      TextDocument.list.find(function(document) {
        return document.fileName.toLocaleLowerCase() === canonicalFileName;
      }));
}

/**
 * @this {!TextDocument}
 */
function close() {
  const document = this;
  if (!document.needSave()) {
    document.forceClose();
    return;
  }
  Editor
      .messageBox(
          null,
          Editor.localizeText(Strings.IDS_ASK_SAVE, {name: document.name}),
          MessageBox.ICONWARNING | MessageBox.YESNOCANCEL)
      .then(function(responseCode) {
        switch (responseCode) {
          case DialogItemId.NO:
            document.forceClose();
            break;
          case DialogItemId.YES:
            Editor.getFileNameForSave(null, document.fileName)
                .then(function(fileName) {
                  document.save(fileName).then(function() {
                    document.forceClose();
                  });
                });
            break;
        }
      });
}

/**
 * @this {!TextDocument}
 */
function forceClose() {
  this.listWindows().forEach(function(window) { window.destroy(); });
  TextDocument.remove(this);
};

/**
 * @param {string} fileName A backing store file of document.
 * @return {!TextDocument} A TextDocument bound to fileName
 */
function open(fileName) {
  const absoluteFileName = FilePath.fullPath(fileName);
  const present = TextDocument.findFile(absoluteFileName);
  if (present)
    return present;
  const document = TextDocument.new(FilePath.basename(fileName));
  document.fileName = absoluteFileName;
  document.mode = Mode.chooseModeByFileName(fileName);
  return document;
}

/**
 * @this {!TextDocument}
 * @return {boolean}
 */
function needSave() {
  // TODO(eval1749): We should use |document.notForSave|.
  return this.modified && !this.name.startsWith('*') &&
      FilePath.isValidFileName(this.fileName);
}

/**
 * @this {!TextDocument}
 * This function handles Emacs "File Variables" in the first line.
 * TODO(eval1749): Support "Local Variables: ... End:".
 */
function parseFileProperties() {
  const document = this;
  const firstLine = new TextRange(document);
  firstLine.endOf(Unit.LINE, Alter.EXTEND);
  const fileVars_matches = /-\*-\s+(.+?)\s+-\*-/.exec(firstLine.text);
  if (!fileVars_matches)
    return;
  fileVars_matches[1].split(';').forEach(function(varDef) {
    let matches = /^\s*([^:\s]+)\s*:\s*(.+?)\s*$/.exec(varDef);
    if (!matches)
      return;
    document.properties.set(matches[1], matches[2]);
  });
};

Object.defineProperties(TextDocument.prototype, {
  lastStatTime_: {value: new Date(0), writable: true},
  obsolete: {value: TextDocument.Obsolete.UNKNOWN, writable: true},
});

[close, forceClose, needSave, parseFileProperties].forEach(
    fn => Object.defineProperty(TextDocument.prototype, fn.name, {value: fn}));

[findFile, open].forEach(
    fn => Object.defineProperty(TextDocument, fn.name, {value: fn}));
});
