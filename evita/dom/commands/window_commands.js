// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  /**
   * @param {string} absoluteFileName
   * @return {!Document}
   */
  function openFile(absoluteFileName) {
    var document = Document.open(absoluteFileName);
    if (!document.length) {
      document.load(absoluteFileName).catch(function(error_code) {
        console.log('Load error', error_code);
        Editor.messageBox(null, 'Failed to load ' + absoluteFileName + '\n' +
            'error=' + error_code,
            MessageBox.ICONERROR);
      }).catch(JsConsole.errorHandler);
    }
    return document;
  }

  /**
   * Open new document in new window in current editor window.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+N', function(arg) {
    var editorWindow = this.parent;
    if (arguments.length >= 1) {
      windows.newTextWindow(editorWindow, new Document('untitled.txt'));
      return;
    }

    Editor.getFileNameForSave(this, this.selection.document.fileName)
        .then(function(fileName) {
          var document = Document.open(fileName);
          windows.newTextWindow(editorWindow, document)
        }).catch(JsConsole.errorHandler);
  });

  /**
   * Open document in new or existing window in current editor window.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+O', function() {
    var editorWindow = this.parent;
    Editor.getFileNameForLoad(this, this.selection.document.fileName)
        .then(function(fileName) {
          windows.activate(editorWindow, openFile(fileName));
        }).catch(JsConsole.errorHandler);
  });

  /**
   * Close this window.
   * @this {!Window}
   * Note:
   *   If Windows doesn't generate key combination Ctrl+Shift+0, please
   *   check "Advanced Key Settings" of "Text Services and Input Languages"
   *   of "Regional and Language Options". If "Ctrl+Shift" key sequence
   *   is assigned, turn this assignment off. This may fix this problem.
   *
   *   In this case, Windows generates WM_INPUTLANGCHANGEREQUEST for
   *   Ctrl+Shift sequence.
   */
  Editor.bindKey(Window, 'Ctrl+Shift+0', function() {
    if (!this.nextSibling && !this.previousSibling) {
      Editor.messageBox(this,
          Editor.localizeText(Strings.IDS_NO_OTHER_WINDOWS),
          MessageBox.ICONWARNING);
      return;
    }
    this.destroy();
  });

  /**
   * Close all windows but this in current editor window.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+Shift+1', function() {
    this.parent.children.forEach(function(window) {
      if (window === this || !window.visible)
        return;
      window.destroy();
    }, this);
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
   * Close all editor window but this.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+Shift+9', function() {
    var this_editor_window = this.parent;
    EditorWindow.list.forEach(function(editor_window) {
      if (editor_window != this_editor_window)
        editor_window.destroy();
    });
  });

  /**
   * Open new document in new editor window.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+Shift+N', function(arg) {
    if (arguments.length >= 1) {
      windows.newEditorWindow(new Document('untitled.txt'));
      return;
    }

    Editor.getFileNameForSave(this, this.selection.document.fileName)
        .then(function(fileName) {
          var document = Document.open(fileName);
          windows.newEditorWindow(document);
        }).catch(JsConsole.errorHandler);
  });

  /**
   * Open document in new editor window.
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+Shift+O', function() {
    Editor.getFileNameForLoad(this, this.selection.document.fileName)
        .then(function(fileName) {
          windows.newEditorWindow(openFile(fileName));
        }).catch(JsConsole.errorHandler);
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
   * @this {!Window}
   */
  Editor.bindKey(Window, 'Ctrl+Shift+W', function(arg) {
    if (arg)
      Editor.forceExit();
    else
      Editor.exit();
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
