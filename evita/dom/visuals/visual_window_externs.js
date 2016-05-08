// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @type {!visuals.Document} */
VisualWindow.prototype.document;

/** @type {!css.CSSStyleSheet} */
VisualWindow.prototype.styleSheet;

/**
 * @this {!VisualWindow}
 * @param {number} pointX
 * @param {number} pointY
 * @return {visuals.Node}
 */
VisualWindow.prototype.hitTest = function(pointX, pointY) {};

/**
 * @param {!visuals.Document} document
 * @param {!css.CSSStyleSheet} styleSheet
 * @return {!VisualWindow}
 */
VisualWindow.newWindow = function(document, styleSheet) {};
