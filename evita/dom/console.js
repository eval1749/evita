// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

var console = {
  DOCUMENT_NAME: '*console log*',

  /**
   * Clear console log contents.
   */
  clear: function() {
    var document = console.document_();
    var range = new Range(document, 0, document.length);
    range.text = '';
  },

  /**
   * @private
   * @return {Document}
   */
  document_: function() {
    return Document.getOrNew(console.DOCUMENT_NAME);
  },

  /**
   * Output arguments to console log.
   * @param {...Object}
   */
  log: function() {
    var message = Array.prototype.slice.call(arguments, 0).map(function(arg) {
      try {
        return arg.toString();
      } catch (e) {
        return e.toString();
      }
    }).join(' ');
    var document = console.document_();
    var range = new Range(document);
    range.endOf(Unit.DOCUMENT).text = message + '\n';
  },

  /**
   * Active or create window to show console log.
   */
  show: function() {
    var document = console.document_();
    var windows = document.listWindows();
    if (windows.length) {
      windows[0].focus();
      return;
    }
    var editorWindow = new EditorWindow();
    var window = new TextWindow(new Range(console.document_()));
    editorWindow.add(window);
    editorWindow.realize();
  }
};
