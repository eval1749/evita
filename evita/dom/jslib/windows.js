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

  /**
   * @param {!Window} current
   * @return {?Window}
   */
  function nextWindow(current) {
    if (current.nextSibling)
      return current.nextSibling;
    var windows = EditorWindow.list;
    var index = windows.findIndex(function(window) {
      return window === current.parent;
    });
    if (windows[index + 1])
      return windows[index + 1].firstChild;
    return null;
  }

  /**
   * @param {!Window} current
   * @return {?Window}
   */
  function previousWindow(current) {
    if (current.previousSibling)
      return current.previousSibling;
    var windows = EditorWindow.list;
    var index = windows.findIndex(function(window) {
      return window === current.parent;
    });
    if (index > 0 && windows[index - 1])
      return windows[index - 1].firstChild;
    return null;
  }

  return {
    activate: {value: activate},
    newEditorWindow: {value: newEditorWindow},
    newTextWindow: {value: newTextWindow},
    nextWindow: {value: nextWindow},
    previousWindow: {value: previousWindow}
  };
})());
