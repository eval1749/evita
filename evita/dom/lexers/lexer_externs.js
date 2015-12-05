// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @typedef {{
 *  characters: !Map.<number, !symbol>,
 *  keywords: !Map.<string, string>
 * }}
 */
var LexerOptions;

/**
 * @constructor
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
Lexer.State;

/** @const @type {!Lexer.State} */
Lexer.State.DOT;

/** @const @type {!Lexer.State} */
Lexer.State.LINE_COMMENT;

/** @const @type {!Lexer.State} */
Lexer.State.OPERATOR;

/** @const @type {!Lexer.State} */
Lexer.State.OTHER;

/** @const @type {!Lexer.State} */
Lexer.State.SPACE;

/** @const @type {!Lexer.State} */
Lexer.State.STRING1;

/** @const @type {!Lexer.State} */
Lexer.State.STRING1_END;

/** @const @type {!Lexer.State} */
Lexer.State.STRING1_ESCAPE;

/** @const @type {!Lexer.State} */
Lexer.State.STRING2;

/** @const @type {!Lexer.State} */
Lexer.State.STRING2_END;

/** @const @type {!Lexer.State} */
Lexer.State.STRING2_ESCAPE;

/** @const @type {!Lexer.State} */
Lexer.State.ZERO;

/**
 * @constructor
 * @param {!Lexer.State} state
 * @param {number} start
 */
Lexer.Token = function(state, start) {};

/** @type {?Object} */
Lexer.Token.prototype.data;

/** @type {number} */
Lexer.Token.prototype.end;

/** @type {number} */
Lexer.Token.prototype.start;

/** @type {!Lexer.State} */
Lexer.Token.prototype.state;

/** @type {string} */
Lexer.Token.prototype.type;

/**
 * @type {!function(!Array.<string>): !Map.<string, string>}
 */
Lexer.createKeywords;

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
 * @type {?Lexer.Token}
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
 * TODO(eval1749): Closure compiler doesn't recognize
 * OrderedSetNode<Lexer.Token>.prototype.previous.
 * @type {?} !OrderedSet.<!Lexer.Token>
 */
Lexer.prototype.tokens;

/**
 * @param {number} offset
 */
Lexer.prototype.adjustScanOffset = function(offset) {};

/**
 * @type {!function()}
 */
Lexer.prototype.clear = function() {};

/**
 * @param {OrderedSetNode.<!Lexer.Token>} itDelimiter
 * @param {!Lexer.Token} token
 * @return {!Array.<!Lexer.Token>}
 */
Lexer.prototype.collectTokens = function(itDelimiter, token) {};

/**
 * @type {!function()}
 */
Lexer.prototype.colorLastToken = function() {};

/**
 * @param {!Lexer.Token} token
 */
Lexer.prototype.colorToken = function(token) {};

/**
 * @type {!function()}
 */
Lexer.prototype.detach = function() {};

/**
 * @param {!Lexer.Token} token
 * @return {!Lexer.State}
 */
Lexer.prototype.didShrinkLastToken = function(token) {};

/**
 * @param {number} hint
 * @return {number}
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
 * @param {!Lexer.State} nextState
 */
Lexer.prototype.finishToken = function(nextState) {};

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
 * @return {?Lexer.Token}
 */
Lexer.prototype.nextToken = function(maxOffset) {};

/**
 * @param {!Lexer.State} newState
 */
Lexer.prototype.restartToken = function(newState) {};

/**
 * @param {!Lexer.State} state
 */
Lexer.prototype.startToken = function(state) {};

/**
 * @param {!Range} range
 * @param {!Lexer.Token} token
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
 * @param {!Array.<!Lexer.Token>} tokens
 * @return {string}
 */
Lexer.prototype.syntaxOfTokens = function(range, tokens) {};

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
