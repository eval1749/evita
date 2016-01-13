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
    return VisualWindow.newWindow_(document.handle_, styleSheet.handle_);
  }

  Object.defineProperties(VisualWindow.prototype, {
    hitTest: {value: hitTest},
  });

  Object.defineProperties(VisualWindow, {
    handleEvent: {value: handleEvent},
    newWindow: {value: newWindow},
  });
})();
