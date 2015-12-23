// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @type {!Object} */
var lexers;

/**
 * @typedef {{
 *  characters: !Map.<number, !symbol>,
 *  keywords: !Map.<string, string>
 * }}
 */
var LexerOptions;

/**
 * @constructor
 * @extends {text.SimpleMutationObserverBase}
 * @implements {Runnable}
 * @param {!Document} document
 * @param {!LexerOptions} options
 * @return {undefined}
 */
function Lexer(document, options) {}

/** @const @type {!symbol} */
Lexer.DOT_CHAR;

/** @const @type {!symbol} */
Lexer.NAMESTART_CHAR;

/** @const @type {!symbol} */
Lexer.NAME_CHAR;

/** @const @type {!symbol} */
Lexer.OPERATOR_CHAR;

/** @const @type {!symbol} */
Lexer.OTHER_CHAR;

/** @const @type {!symbol} */
Lexer.STRING1_CHAR;

/** @const @type {!symbol} */
Lexer.STRING2_CHAR;

/** @const @type {!symbol} */
Lexer.STRING3_CHAR;

/** @const @type {!symbol} */
Lexer.WHITESPACE_CHAR;

/** @typedef {!symbol} */
lexers.State;

/** @const @type {!lexers.State} */
lexers.State.DOT;

/** @const @type {!lexers.State} */
lexers.State.LINE_COMMENT;

/** @const @type {!lexers.State} */
lexers.State.OPERATOR;

/** @const @type {!lexers.State} */
lexers.State.OTHER;

/** @const @type {!lexers.State} */
lexers.State.SPACE;

/** @const @type {!lexers.State} */
lexers.State.STRING1;

/** @const @type {!lexers.State} */
lexers.State.STRING1_END;

/** @const @type {!lexers.State} */
lexers.State.STRING1_ESCAPE;

/** @const @type {!lexers.State} */
lexers.State.STRING2;

/** @const @type {!lexers.State} */
lexers.State.STRING2_END;

/** @const @type {!lexers.State} */
lexers.State.STRING2_ESCAPE;

/** @const @type {!lexers.State} */
lexers.State.ZERO;

/**
 * @constructor
 * @param {!lexers.State} state
 * @param {number} start
 */
lexers.Token = function(state, start) {};

/** @type {?Object} */
lexers.Token.prototype.data;

/** @type {number} */
lexers.Token.prototype.end;

/** @type {number} */
lexers.Token.prototype.start;

/** @type {!lexers.State} */
lexers.Token.prototype.state;

/** @type {string} */
lexers.Token.prototype.type;

/**
 * @param {!Iterable<string>|!Array<string>} keywords
 * @return {!Map.<string, string>}
 */
Lexer.createKeywords = function(keywords) {};

/**
 * @type {!Map.<number, !symbol>}
 */
Lexer.prototype.characters_;

/**
 * @type {number}
 */
Lexer.prototype.count;

/**
 * @type {number}
 */
Lexer.prototype.debug_;

/**
 * @type {!Document}
 */
Lexer.prototype.document;

/**
 * @type {!Map.<string, string>}
 */
Lexer.prototype.keywords;

/**
 * @type {number}
 */
Lexer.prototype.maxChainWords_;

/**
 * @type {!MutationObserver}
 */
Lexer.prototype.mutationObserver_;

/**
 * @type {!lexers.Token}
 */
Lexer.prototype.lastToken;

/**
 * @type {?Lexer}
 */
Lexer.prototype.parentLexer_;

/**
 * @type {?Range}
 */
Lexer.prototype.range;

/**
 * @type {number}
 */
Lexer.prototype.scanOffset;

/**
 * @type {!symbol}
 */
Lexer.prototype.state;

/**
 * @type {?Object}
 */
Lexer.prototype.tokenData;

/**
 * @type {!base.OrderedSet<!lexers.Token>}
 */
Lexer.prototype.tokens;

/**
 * @param {number} offset
 */
Lexer.prototype.adjustScanOffset = function(offset) {};

/**
 * @param {!lexers.Token} token
 * @param {string} type
 */
Lexer.prototype.changeTokenType = function(token, type) {};

/**
 * @type {!function()}
 */
Lexer.prototype.clear = function() {};

/**
 * @param {base.OrderedSetNode<!lexers.Token>} itDelimiter
 * @param {!lexers.Token} token
 * @return {!Array.<!lexers.Token>}
 */
Lexer.prototype.collectTokens = function(itDelimiter, token) {};

/**
 * @param {!lexers.Token} token
 */
Lexer.prototype.colorToken = function(token) {};

/**
 * @type {!function()}
 */
Lexer.prototype.detach = function() {};

/**
 * @param {!lexers.Token} token
 * @return {!lexers.State}
 */
Lexer.prototype.didShrinkLastToken = function(token) {};

/**
 * @param {!lexers.Token} token
 */
Lexer.prototype.didEndToken = function(token) {};

/**
 * @param {number} hint
 */
Lexer.prototype.doColor = function(hint) {};

/**
 * @type {!function()}
 */
Lexer.prototype.endToken = function() {};

/**
 * @type {!function()}
 */
Lexer.prototype.extendToken = function() {};

/**
 * @param {number} charCode
 */
Lexer.prototype.feedCharacter = function(charCode) {};

/**
 * @param {!lexers.State} nextState
 */
Lexer.prototype.finishState = function(nextState) {};

/**
 * @this {!Lexer}
 * @param {number} charCode
 * @return {boolean}
 */
Lexer.prototype.isNameChar = function(charCode) {};

/**
 * @this {!Lexer}
 * @param {number} charCode
 * @return {boolean}
 */
Lexer.prototype.isNameStartChar = function(charCode) {};

/**
 * @this {!Lexer}
 * @param {number} charCode
 * @return {boolean}
 */
Lexer.prototype.isOtherChar = function(charCode) {};

/**
 * @this {!Lexer}
 * @param {number} charCode
 * @return {boolean}
 */
Lexer.prototype.isWhitespaceChar = function(charCode) {};

/**
 * @this {!Lexer}
 * @param {number} charCode
 * @return {boolean}
 */
Lexer.prototype.isWordChar = function(charCode) {};

/**
 * @param {number} maxOffset
 * @return {?lexers.Token}
 */
Lexer.prototype.nextToken = function(maxOffset) {};

/**
 * @param {!lexers.State} newState
 */
Lexer.prototype.restartToken = function(newState) {};

/**
 * @param {!lexers.State} state
 */
Lexer.prototype.startToken = function(state) {};

/**
 * @param {!Range} range
 * @param {!lexers.Token} token
 * @return {string}
 */
Lexer.prototype.syntaxOfToken = function(range, token) {};

/**
 * @this {!Lexer}
 * @param {string} word
 * @return {string}
 */
Lexer.prototype.syntaxOfWord = function(word) {};

/**
 * @this {!Lexer}
 * @param {!Range} range
 * @param {!Array.<!lexers.Token>} tokens
 * @return {string}
 */
Lexer.prototype.syntaxOfTokens = function(range, tokens) {};

/**
 * @this {!Lexer}
 * @param {!lexers.Token} token
 * @return {string}
 */
Lexer.prototype.tokenTextof = function(token) {};

/**
 * TODO(eval1749): Once, Closure compiler recognizes |ClikeLexer| is subclass of
 * |Lexer|, we enable |@protected|.
 * protected
 * @param {number} charCode
 */
Lexer.prototype.updateState = function(charCode) {};

// TODO(eval1749): We should delete below line once closure compiler fixed.
/** @typedef {Lexer} */
global.Lexer;
