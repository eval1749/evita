// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  /**
   * @param {EditorWindow} source
   * @param {EditorWindow} target
   */
  function handleDropWindow(source, target) {
    // TODO(eval1749): Realize target without showing then show target after
    // change parent.
    if (target.state == Window.State.NOTREALIZED)
      target.realize();
    source.changeParent(target);
  }

  /**
   * @param {EditorWindow} window
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
   * @param {Event} event
   */
 EditorWindow.handleEvent = function(event) {
   switch (event.type) {
     case Event.Names.BLUR:
     case Event.Names.FOCUS:
       break;
     case Event.Names.DROPWINDOW: {
       var windowEvent = /** @type {!WindowEvent} */(event);
       var editorWindow = /** @type {!EditorWindow} */(
          windowEvent.sourceWindow);
       handleDropWindow(editorWindow, this);
       break;
     }
     case Event.Names.QUERYCLOSE:
       handleQueryClose(this);
       break;
     default:
       Window.handleEvent.call(this, event);
       break;
   }
 };

  /**
   * Open file in window.
   * @param {string} fileName
   *
   * This function handles drag-and-drop.
   */
  EditorWindow.prototype.open = function(fileName) {
     var document = Document.open(fileName);
     var window = this.children.find(function(present) {
       return present.document === document;
     });
     if (window) {
       window.focus();
       return;
     }
     document.load()
     var new_window = new TextWindow(new Range(document));
     this.appendChild(new_window);
   };
})();
