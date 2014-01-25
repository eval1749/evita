// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

global.editors = {};

/** @param {!Array.<string>} args */
editors.start = function(args) {
  var doc = new Document('*scratch*');
  var range = new Range(doc);
  var editor_window = new EditorWindow();
  var text_window = new TextWindow(range);
  editor_window.appendChild(text_window);
  editor_window.realize();
};
