// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @typedef {Object}
 *  characters: {!Map<number, !Symbol},
 *  hasCpp: boolean,
 *  keywords: {!Iterable.<string>} keywords
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
 * @return {!Map.<number, number>}
 */
ClikeLexer.newCharacters = function() {};

/** @const @type {!Symbol} */
ClikeLexer.COLON_CHAR;

/** @const @type {!Symbol} */
ClikeLexer.DOT_CHAR;

/** @const @type {!Symbol} */
ClikeLexer.SLASH_CHAR;

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

/**
 * @constructor
 * @extends {ClikeLexer}
 * @param {!Document} document
 * @return {undefined}
 */
function PythonLexer(document) {}

/**
 * @constructor
 * @extends {Lexer}
 * @param {!Document} document
 * @return {undefined}
 */
function XmlLexer(document) {}
