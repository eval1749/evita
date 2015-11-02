// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @typedef {Object}
 *  characters: {!Map<number, !symbol},
 *  hasCpp: boolean,
 *  keywords: {!Set.<string>} keywords
 */
var ClikeLexerOptions;

/**
 * @constructor
 * @extends {Lexer}
 * @param {!Document} document
 * TODO(eval1749): We should use |ClikeLexerOptions| instead of |Object| for
 * |options|.
 * @param {!Object} options
 * @return {undefined}
 */
function ClikeLexer(document, options) {}

/**
 * @return {!Map.<number, number>}
 */
ClikeLexer.newCharacters = function() {};

/** @type {!symbol} */
ClikeLexer.COLON_CHAR;

/** @type {!symbol} */
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
 * @extends {Lexer}
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
function CsharpLexer(document) {}

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
 * @param {!Lexer=} opt_parentLexer
 * @return {undefined}
 */
function JavaScriptLexer(document, opt_parentLexer) {}

/**
 * @constructor
 * @extends {Lexer}
 * @param {!Document} document
 * @return {undefined}
 */
function PythonLexer(document) {}

/**
 * @typedef {Object}
 *   childLexers: !Object
 *   ignoreCase: boolean,
 *   keywords: !Iterable
 */
var XmlLexerOptions;

/**
 * @constructor
 * @extends {Lexer}
 * @param {!Document} document
 * TODO(eval1749): We should use |XmlLexerOptions| instead of |Object| for
 * |options|.
 * @param {!Object=} opt_options
 * @return {undefined}
 */
function XmlLexer(document, opt_options) {}

/**
 * @constructor
 * @extends {XmlLexer}
 * @param {!Document} document
 * @return {undefined}
 */
function HtmlLexer(document) {}

// TODO(eval1749): We should delete below line once closure compiler fixed.
/** @typedef {ClikeLexer} */
global.ClikeLexer;

/** @typedef {XmlLexer} */
global.XmlLexer;

