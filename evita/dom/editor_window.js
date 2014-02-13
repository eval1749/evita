// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  /**
   * @param {EditorWindow} source.
   * @param {EditorWindow} target.
   */
  function handleDropWindow(source, target) {
    // TODO(yosi) Realize target without showing then show target after
    // change parent.
    if (target.state == Window.State.NOTREALIZED)
      target.realize();
    source.changeParent(target);
  }

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
     case Event.Names.DROPWINDOW: {
       var window_event = /** @type{WindowEvent} */(event);
       handleDropWindow(window_event.sourceWindow, this);
       break;
     }
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
   *
   * This function handles drag-and-drop.
   */
  EditorWindow.prototype.open = function(filename) {
     var document = Document.open(filename);
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
