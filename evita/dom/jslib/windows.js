// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @const @type{{
 *  activate: function(!Window, !Document),
 *  newEditorWindow: function(!Document),
 *  newTextWindow: function(!Window, !Document)
 * }}
 */
global.windows = {
  /**
   * @param {!Window} parent
   * @param {!Document} document
   */
  activate: function(parent, document) {
    var present = parent.children.find(function(window) {
      return window instanceof TextWindow && window.document == document;
    });
    if (present) {
      present.focus();
      return;
    }
    windows.newTextWindow(parent, document);
  },

  /**
   * @param {!Document} document
   */
  newEditorWindow: function(document) {
    var editor_window = new EditorWindow();
    windows.newTextWindow(editor_window, document);
    editor_window.realize();
  },

  /**
   * @param {!Window} parent
   * @param {!Document} document
   */
  newTextWindow: function(parent, document) {
    parent.appendChild(new TextWindow(new Range(document)));
  }
};
