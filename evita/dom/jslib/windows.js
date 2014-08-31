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
   * @return {?Window}
   */
  function firstWindow() {
    var editorWindow = EditorWindow.list[0];
    return editorWindow ? editorWindow.firstChild : null;
  }

  /**
   * @return {?Window}
   */
  function lastWindow() {
    var editorWindows = EditorWindow.list;
    if (!editorWindows.length)
      return null;
    var editorWindow = editorWindows[editorWindows.length - 1];
    return editorWindows ? editorWindow.lastChild : null;
  }

  /**
   * @param {!Document} document
   */
  function newEditorWindow(document) {
    var editorWindow = new EditorWindow();
    windows.newTextWindow(editorWindow, document);
    editorWindow.realize();
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
    var editorWindows = EditorWindow.list;
    var index = editorWindows.findIndex(function(window) {
      return window === current.parent;
    });
    var editorWindow = editorWindows[index + 1];
    return editorWindow ? editorWindow.firstChild : null;
  }

  /**
   * @param {!Window} current
   * @return {?Window}
   */
  function previousWindow(current) {
    if (current.previousSibling)
      return current.previousSibling;
    var editorWindows = EditorWindow.list;
    var index = editorWindows.findIndex(function(window) {
      return window === current.parent;
    });
    var editorWindow = editorWindows[index - 1];
    return editorWindow ? editorWindow.lastChild : null;
  }

  return {
    activate: {value: activate},
    firstWindow: {value: firstWindow},
    lastWindow: {value: lastWindow},
    newEditorWindow: {value: newEditorWindow},
    newTextWindow: {value: newTextWindow},
    nextWindow: {value: nextWindow},
    previousWindow: {value: previousWindow}
  };
})());
