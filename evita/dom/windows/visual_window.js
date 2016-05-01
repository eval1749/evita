// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {
/**
 * @param {!Event} event
 */
function handleEvent(event) {}

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
  const window = VisualWindow.newWindow_(document.handle_, styleSheet.handle_);
  window.document_ = document;
  window.styleSheet_ = styleSheet;
  return window;
}

/**
 * @this {!VisualWindow}
 * @return {!TextDocument}
 */
function getDocument() {
  return this.document_;
}

/**
 * @this {!VisualWindow}
 * @return {!CSSStyleSheet}
 */
function getStyleSheet() {
  return this.styleSheet_;
}

Object.defineProperties(VisualWindow.prototype, {
  document_: {value: null, writable: true},
  document: {get: getDocument},
  hitTest: {value: hitTest},
  styleSheet_: {value: null, writable: true},
  styleSheet: {get: getStyleSheet},
});

Object.defineProperties(VisualWindow, {
  handleEvent: {value: handleEvent},
  newWindow: {value: newWindow},
});

/** @type {!TextDocument} */
VisualWindow.prototype.document;

/** @type {!CSSStyleSheet} */
VisualWindow.prototype.styleSheet;

/**
 * @param {number} point_x
 * @param {number} point_y
 * @return {Node}
 */
VisualWindow.prototype.hitTest;

/**
 * @param {!Document} document
 * @param {!CSSStyleSheet} styleSheet
 * @return {!VisualWindow}
 */
VisualWindow.newWindow;
});
