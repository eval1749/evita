// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  'use strict';

  /**
   * @param {string} absoluteFileName
   * @return {!Document}
   */
  function openFile(absoluteFileName) {
    const document = Document.open(absoluteFileName);
    if (!document.length) {
      document.load(absoluteFileName).catch(function(errorCode) {
        console.log('Load error', errorCode);
        Editor.messageBox(null, 'Failed to load ' + absoluteFileName + '\n' +
            'error=' + errorCode,
            MessageBox.ICONERROR);
      });
    }
    return document;
  }

  /**
   * Open new document in new window in current editor window.
   * @this {!Window}
   */
  function newDocumentCommand(arg) {
    if (!this.parent)
      return;
    const editorWindow = /** @type {!Window} */(this.parent);
    if (arg !== undefined) {
      windows.newTextWindow(editorWindow, Document.new('untitled.txt'));
      return;
    }

    Editor.getFileNameForSave(this, this.selection.document.fileName)
        .then(function(fileName) {
          const document = Document.open(fileName);
          windows.newTextWindow(editorWindow, document)
        });
  }
  Editor.bindKey(Window, 'Ctrl+N', newDocumentCommand);

  /**
   * Open document in new or existing window in current editor window.
   * @this {!Window}
   */
  function openDocumentCommand() {
    if (!this.parent)
      return;
    const editorWindow = /** @type {!Window} */(this.parent);
    Editor.getFileNameForLoad(this, this.selection.document.fileName)
        .then(function(fileName) {
          windows.activate(editorWindow, openFile(fileName));
        });
  }
  Editor.bindKey(Window, 'Ctrl+O', openDocumentCommand);

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
  function closeThisWindowCommand() {
    const nextFocus = windows.nextWindow(this) || windows.previousWindow(this);
    if (!nextFocus) {
      Editor.messageBox(this,
          Editor.localizeText(Strings.IDS_NO_OTHER_WINDOWS),
          MessageBox.ICONWARNING);
      return;
    }
    nextFocus.focus();
    this.destroy();
  }
  Editor.bindKey(Window, 'Ctrl+Shift+0', closeThisWindowCommand);

  /**
   * Close all windows but this in current editor window.
   * @this {!Window}
   */
  function closeAllWindowsButThisCommand() {
    this.parent.children.forEach(function(window) {
      if (window === this || !window.visible)
        return;
      window.destroy();
    }, this);
  }
  Editor.bindKey(Window, 'Ctrl+Shift+1', closeAllWindowsButThisCommand);

  /**
   * Split window vertically and put new window below.
   * @this {!Window}
   */
  function splitThisWindowVerticallyCommand() {
    if (!this.clone)
      return;
    this.splitVertically(this.clone());
  }
  Editor.bindKey(Window, 'Ctrl+Shift+2', splitThisWindowVerticallyCommand);

  /**
   * Split window horizontally and put new window right.
   * @this {!Window}
   */
  function splitThisWindowHorizontallyCommand() {
    if (!this.clone)
      return;
    this.splitHorizontally(this.clone());
  }
  Editor.bindKey(Window, 'Ctrl+Shift+5', splitThisWindowHorizontallyCommand);

  /**
   * Close all editor window but this.
   * @this {!Window}
   */
  function closeAllEditorWindowsButThis() {
    const thisEditor_window = this.parent;
    EditorWindow.list.forEach(function(editorWindow) {
      if (editorWindow !== thisEditor_window)
        editorWindow.destroy();
    });
  }
  Editor.bindKey(Window, 'Ctrl+Shift+9', closeAllEditorWindowsButThis);

  /**
   * Open new document in new editor window.
   * @this {!Window}
   */
  function newDocumentInNewWindowCommand(arg) {
    if (arg !== undefined) {
      windows.newEditorWindow(Document.new('untitled.txt'));
      return;
    }

    Editor.getFileNameForSave(this, this.selection.document.fileName)
        .then(function(fileName) {
          const document = Document.open(fileName);
          windows.newEditorWindow(document);
        });
  }
  Editor.bindKey(Window, 'Ctrl+Shift+N', newDocumentInNewWindowCommand);

  /**
   * Open document in new editor window.
   * @this {!Window}
   */
  function openDocumentInNewWindowCommand() {
    Editor.getFileNameForLoad(this, this.selection.document.fileName)
        .then(function(fileName) {
          windows.newEditorWindow(openFile(fileName));
        });
  }
  Editor.bindKey(Window, 'Ctrl+Shift+O', openDocumentInNewWindowCommand);

  /**
   * Previous window
   * @this {!Window}
   */
  function previousWindowCommand() {
    const previousWindow = windows.previousWindow(this);
    if (previousWindow) {
      previousWindow.focus();
      return;
    }
    const lastWindow = windows.lastWindow();
    if (lastWindow === this)
      return;
    lastWindow.focus();
  }
  Editor.bindKey(Window, 'Ctrl+Shift+Tab', previousWindowCommand);

  /**
   * @this {!Window}
   */
  function quitCommand(arg) {
    if (arg)
      Editor.forceExit();
    else
      Editor.exit();
  }
  Editor.bindKey(Window, 'Ctrl+Shift+W', quitCommand);

  /**
   * Next window
   * @this {!Window}
   */
  function nextWindowCommand() {
    const nextWindow = windows.nextWindow(this);
    if (nextWindow) {
      nextWindow.focus();
      return;
    }
    const firstWindow = windows.firstWindow();
    if (firstWindow === this)
      return;
    firstWindow.focus();
  }
  Editor.bindKey(Window, 'Ctrl+Tab', nextWindowCommand);
})();
