// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  /**
   * Open new document in new window in current editor window.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+N', function(arg) {
    var editorWindow = this.parent;
    if (arg !== undefined) {
      windows.newTextWindow(editorWindow, new Document('untitled.txt'));
      return;
    }

    Editor.getFilenameForSave(this, this.selection.document.filename)
        .then(function(filename) {
          var document = new Document(FilePath.basename(filename));
          document.filename = filename;
          windows.newTextWindow(editorWindow, document)
        });
  });

  /**
   * Split window vertically and put new window below.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+Shift+2', function() {
    if (!this.clone)
      return;
    this.splitVertically(this.clone());
  });

  /**
   * Split window horizontally and put new window right.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+Shift+5', function() {
    if (!this.clone)
      return;
    this.splitHorizontally(this.clone());
  });

  /**
   * Open new document in new editor window.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+Shift+N', function(arg) {
    if (arg !== undefined) {
      windows.newEditorWindow(new Document('untitled.txt'));
      return;
    }

    Editor.getFilenameForSave(this, this.selection.document.filename)
        .then(function(filename) {
          windows.newEditorWindow(new Document(filename));
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
          windows.newTextWindow(editorWindow, Document.load(filename));
        });
  });

  /**
   * Open document in new editor window.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+Shift+O', function() {
    Editor.getFilenameForLoad(this, this.selection.document.filename)
        .then(function(filename) {
          windows.newEditorWindow(Document.load(filename));
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

  /**
   * Previous window
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+Shift+Tab', function() {
    if (this.previousSibling) {
      this.previousSibling.focus();
      return;
    }

    if (this.parent.previousSibling) {
      this.parent.previousSibling.lastChild.focus();
      return;
    }

    var top_level_windows = EditorWindow.list;
    top_level_windows[top_level_windows.length - 1].lastChild.focus();
  });

  /**
   * Next window
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+Tab', function() {
    if (this.nextSibling) {
      this.nextSibling.focus();
      return;
    }

    if (this.parent.nextSibling) {
      this.parent.nextSibling.firstChild.focus();
      return;
    }

    EditorWindow.list[0].firstChild.focus();
  });
})();
