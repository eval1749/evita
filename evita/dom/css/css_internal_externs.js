// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @param {!Map<number, string>} rawStyle
 * @param {!css.CSSStyleObserver} observer
 * @return {!css.CSSStyleDeclaration}
 */
css.CSSStyleDeclaration.createInternal = function(rawStyle, observer) {};

// TODO(eval1749): Once we fix reference in visuals.Element, we should not
// expose CSSStyleObserver#rawStyle_
/** @const @type {!Map<number, string>} */
css.CSSStyleDeclaration.prototype.rawStyle_;

// TODO(eval1749): Once we fix reference in VisualWindow, we should not
// expose CSSStyleObserver#rawStyle_
/** @const @type {!CSSStyleSheetHandle} */
css.CSSStyleSheet.prototype.handle_;
