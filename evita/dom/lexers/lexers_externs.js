// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @extends {Lexer}
 * @param {Iterable.<string>} keywords
 * @param {!Document} document
 * @return {undefined}
 */
function ClikeLexer(keywords, document) {}

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
function IdlLexer(document) {}

/**
 * @constructor
 * @extends {ClikeLexer}
 * @param {!Document} document
 * @return {undefined}
 */
function JavaScriptLexer(document) {}
