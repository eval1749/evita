// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  /**
   * @param {!Document} document.
   */
  function newEditorWindow(document) {
    var editorWindow = new EditorWindow();
    newTextWindow(editorWindow, document);
    editorWindow.realize();
  }

  /**
   * @param {!Window} document.
   * @param {!Document} document.
   */
  function newTextWindow(parent, document) {
    parent.add(new TextWindow(new Range(document)));
  }

  /**
   * Open new document in new window in current editor window.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+N', function(arg) {
    var editorWindow = this.parent;
    if (arg !== undefined) {
      newTextWindow(editorWindow, new Document('untitled.txt'));
      return;
    }

    Editor.getFilenameForSave(this, this.selection.document.filename)
        .then(function(filename) {
          newTextWindow(editorWindow, new Document(filename))
        });
  });

  /**
   * Open new document in new editor window.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+Shift+N', function(arg) {
    if (arg !== undefined) {
      newEditorWindow(new Document('untitled.txt'));
      return;
    }

    Editor.getFilenameForSave(this, this.selection.document.filename)
        .then(function(filename) {
          newEditorWindow(new Document(filename));
        });
  });

  /**
   * Open document in new or existing window in current editor window.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+O', function() {
    var editorWindow = this.parent;
    Editor.getFilenameForLoad(this, this.selection.document.filename)
        .then(function(filename) {
          // TODO(yosi) When editorWindow has a window for filename, we should
          // activate it.
          newTextWindow(editorWindow, Document.load(filename));
        });
  });

  /**
   * Open document in new editor window.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+Shift+O', function() {
    Editor.getFilenameForLoad(this, this.selection.document.filename)
        .then(function(filename) {
          newEditorWindow(Document.load(filename));
        });
  });

  /**
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+Shift+W', function(arg) {
    if (arg)
      Editor.forceExit();
    else
      Editor.exit();
  });
})();
