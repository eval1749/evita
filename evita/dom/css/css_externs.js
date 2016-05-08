// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('css');

// TODO(eval1749): Once IDL compiler support emitting |goog.provide()|, we
// should not have |goog.provide('css.CSSStyleDeclaration')| here.
goog.provide('css.CSSStyleDeclaration');

// TODO(eval1749): We should have "CSSStyleObserver.idl".
/**
 * @interface
 */
css.CSSStyleObserver = function() {};

/**
 * @param {!css.CSSStyleDeclaration} style
 */
css.CSSStyleObserver.didChangeCSSStyle = function(style) {};

/** @constructor */
css.Parser = function() {};

/**
 * @param {string} source
 * @return {!css.CSSStyleSheet}
 */
css.Parser.parse = function(source) {};
