// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @typedef {{
 *  characters: !Map.<number, !Symbol>,
 *  keywords: !Iterable.<string>,
 *  stateToSyntax: !Map.<!Symbol, string>
 * }}
 */
var LexerOptions;

/**
 * @constructor
 * @param {!Document} document
 * @param {!LexerOptions} options
 */
function Lexer(document, options) {}

/** @const @type {!Symbol} */
Lexer.OPERATOR_CHAR;

/** @const @type {!Symbol} */
Lexer.OTHER_CHAR;

/** @const @type {!Symbol} */
Lexer.STRING1_CHAR;

/** @const @type {!Symbol} */
Lexer.STRING2_CHAR;

/** @const @type {!Symbol} */
Lexer.WHITESPACE_CHAR;

/** @const @type {!Symbol} */
Lexer.WORD_CHAR;

/** @typedef {!Symbol} */
Lexer.State;

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

/** @type {number} */
Lexer.Token.prototype.end;

/** @type {number} */
Lexer.Token.prototype.start;

/** @type {!Lexer.State} */
Lexer.Token.prototype.state;

/** @type {string} */
Lexer.Token.prototype.type;

/**
 * @type {number}
 */
Lexer.prototype.changeOffset;

/**
 * @type {!Map.<number, number>}
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
 * @type {!Set.<string>}
 */
Lexer.prototype.keywords;

/**
 * @type {!MutationObserver}
 */
Lexer.prototype.mutationObserver_;

/**
 * @type {?Lexer.Token}
 */
Lexer.prototype.lastToken;

/**
 * @type {?Range}
 */
Lexer.prototype.range;

/**
 * @type {number}
 */
Lexer.prototype.scanOffset;

/**
 * @type {!Symbol}
 */
Lexer.prototype.state;

/**
 * @type {!Map.<number,string>}
 */
Lexer.prototype.stateToSyntax_;

/**
 * @type {OrderedSet.<!Lexer.Token>}
 */
Lexer.prototype.tokens;

/**
 * @type {!function()}
 */
Lexer.prototype.adjustScanOffset = function() {};

/**
 */
Lexer.prototype.detach = function() {};

/**
 * @type {!function()}
 */
Lexer.prototype.didChangeScanOffset = function() {};

/**
 * @param {!Lexer.Token} token
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
 * @param {!Range} range
 * @param {!Lexer.Token} token
 * @return {string}
 */
Lexer.prototype.extractWord = function(range, token) {};

/**
 * @param {!Lexer.State} nextState
 */
Lexer.prototype.finishToken = function(nextState) {};

/**
 * @this {!Lexer}
 * @param {number} charCode
 * @return {boolean}
 */
Lexer.prototype.isOperator = function(charCode) {};

/**
 * @this {!Lexer}
 * @param {number} charCode
 * @return {boolean}
 */
Lexer.prototype.isOther = function(charCode) {};

/**
 * @this {!Lexer}
 * @param {number} charCode
 * @return {boolean}
 */
Lexer.prototype.isWhitespace = function(charCode) {};

/**
 * @this {!Lexer}
 * @param {number} charCode
 * @return {boolean}
 */
Lexer.prototype.isWord = function(charCode) {};

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
 * TODO(yosi) Once, Closure compiler recognizes |ClikeLexer| is subclass of
 * |Lexer|, we enable |@protected|.
 * protected
 * @param {number} charCode
 */
Lexer.prototype.updateState = function(charCode) {};
