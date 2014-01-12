// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  /**
   * @param {EditorWindow} window.
   */
  function handleQueryClose(window) {
    if (EditorWindow.list.length == 1) {
      Editor.exit();
      return;
    }
    window.destroy();
  }

  /**
   * Default event handler.
   * @this {EditorWindow}
   * @param {Event} event.
   */
 EditorWindow.handleEvent = function(event) {
   switch (event.type) {
     case Event.Names.QUERYCLOSE:
       handleQueryClose(this);
       break;
     default:
       console.log('EditorWindow.handlEvent received unsupported event',
                   event);
       break;
   }
 };

  /**
   * Open file in window.
   * @param {string} filename.
   */
  EditorWindow.prototype.open = function(filename) {
    var document = Document.load(filename);
    var window = this.children.find(function(present) {
      return present.document === document;
    });
    if (window) {
      window.focus();
      return;
    }
    var new_window = new TextWindow(new Range(document));
    this.add(new_window);
  };
})();
