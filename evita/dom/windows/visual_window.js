// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @param {!Event} event
   */
  function handleEvent(event) {
  }

  /**
   * @this {!VisualWindow}
   * @param {number} point_x
   * @param {number} point_y
   * @return {Node}
   */
  function hitTest(point_x, point_y) {
    /** @const @type {number} */
    const foundId = this.hitTest_(point_x, point_y);
    if (foundId <= 0)
      return null;
    return Node.nodeFromId(foundId);
  }

  /**
   * @param {!Document} document
   * @param {!CSSStyleSheet} styleSheet
   * @return {!VisualWindow}
   */
  function newWindow(document, styleSheet) {
    const window = VisualWindow.newWindow_(document.handle_,
                                           styleSheet.handle_);
    window.document_ = document;
    window.styleSheet_ = styleSheet;
    return window;
  }

  Object.defineProperties(VisualWindow.prototype, {
    document_: {value: null, writable: true},
    document: {get: function() { return this.document_; }},
    hitTest: {value: hitTest},
    styleSheet_: {value: null, writable: true},
    styleSheet: {get: function() { return this.styleSheet_; }},
  });

  Object.defineProperties(VisualWindow, {
    handleEvent: {value: handleEvent},
    newWindow: {value: newWindow},
  });
})();
