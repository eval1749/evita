// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('css');

goog.scope(function() {
/**
 * @interface
 */
var CSSStyleObserver = function() {};

/**
 * @param {!css.CSSStyleDeclaration} style
 */
CSSStyleObserver.didChangeCSSStyle = function(style) {};

/** @interface */
css.CSSStyleObserver = CSSStyleObserver;
});
