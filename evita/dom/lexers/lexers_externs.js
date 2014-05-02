// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @typedef {Object}
 *  keywords: {Iterable.<string>} keywords
 */
var ClikeLexerOptions;

/**
 * @constructor
 * @extends {Lexer}
 * @param {!Document} document
 * TODO(yosi) We should use |ClikeLexerOptions| instead of |Object| for
 * |options|.
 * @param {!Object} options
 * @return {undefined}
 */
function ClikeLexer(document, options) {}

/**
 * @constructor
 * @extends {ClikeLexer}
 * @param {!Document} document
 * @return {undefined}
 */
function CppLexer(document) {}

/**
 * @constructor
 * @extends {ClikeLexer}
 * @param {!Document} document
 * @return {undefined}
 */
function ConfigLexer(document) {}

/**
 * @constructor
 * @extends {ClikeLexer}
 * @param {!Document} document
 * @return {undefined}
 */
function IdlLexer(document) {}

/**
 * @constructor
 * @extends {ClikeLexer}
 * @param {!Document} document
 * @return {undefined}
 */
function JavaLexer(document) {}

/**
 * @constructor
 * @extends {ClikeLexer}
 * @param {!Document} document
 * @return {undefined}
 */
function JavaScriptLexer(document) {}
