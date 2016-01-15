// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @type {!Document} */
VisualWindow.prototype.document;

/** @type {!CSSStyleSheet} */
VisualWindow.prototype.styleSheet;

/**
 * @this {!VisualWindow}
 * @param {number} point_x
 * @param {number} point_y
 * @return {Node}
 */
VisualWindow.prototype.hitTest = function (point_x, point_y) {};

/**
 * @param {!Document} document
 * @param {!CSSStyleSheet} styleSheet
 * @return {!VisualWindow}
 */
VisualWindow.newWindow = function(document, styleSheet) {};
