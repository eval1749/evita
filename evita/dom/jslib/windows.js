// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.windows = Object.create({}, (function() {
  /**
   * @param {!Window} parent
   * @param {!Document} document
   */
  function activate(parent, document) {
    var present = parent.children.find(function(window) {
      return window instanceof TextWindow && window.document == document;
    });
    if (present) {
      present.focus();
      return;
    }
    windows.newTextWindow(parent, document);
  }

  /**
   * @param {!Document} document
   */
  function newEditorWindow(document) {
    var editor_window = new EditorWindow();
    windows.newTextWindow(editor_window, document);
    editor_window.realize();
  }

  /**
   * @param {!Window} parent
   * @param {!Document} document
   */
  function newTextWindow(parent, document) {
    parent.appendChild(new TextWindow(new Range(document)));
  }

  return {
    activate: {value: activate},
    newEditorWindow: {value: newEditorWindow},
    newTextWindow: {value: newTextWindow},
  };
})());
