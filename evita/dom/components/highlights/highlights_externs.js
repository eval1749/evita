// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('highlights');

/** @typedef {function(!TextDocument):!highlights.HighlightEngine} */
highlights.HighlightEngineCreator;

/**
 * @constructor
 * @extends {text.SimpleMutationObserverBase}
 * @param {!TextDocument} document
 * @param {!function(!TextDocument):!highlights.Painter} painterCreator
 * @param {!highlights.TokenStateMachine} stateMachine
 */
highlights.HighlightEngine = function(document, painterCreator, stateMachine) {
};

/** @public */
highlights.HighlightEngine.prototype.detach = function() {};

/**
 * @public
 * @param {number} hint
 */
highlights.HighlightEngine.prototype.doColor = function(hint) {};

/**
 * @param {string} name
 * @param {!TextDocument} document
 * @return {!highlights.HighlightEngine}
 */
highlights.HighlightEngine.createEngine = function(name, document) {};

/**
 * @public
 * @param {string} name
 * @return {!Set<string>}
 */
highlights.HighlightEngine.keywordsFor = function(name) {};

/**
 * @public
 * @param {string} name
 * @param {!highlights.HighlightEngineCreator} creator
 * @param {!Set<string>} keywords
 */
highlights.HighlightEngine.registerEngine = function(name, creator, keywords) {
};

/**
 * @constructor
 * @protected
 * @param {!TextDocument} document
 */
highlights.Painter = function(document) {};

/**
 * @public
 * @param {number} headCount
 * @param {number} tailCount
 * @param {number} delta
 * Exposed for embed language tokenizer.
 */
highlights.Painter.prototype.didChangeTextDocument =
    function(headCount, tailCount, delta) {}

    /**
     * @public
     * Exposed for embed language tokenizer.
     */
    highlights.Painter.prototype.didLoadTextDocument = function() {};

/**
 * @public
 * @type {!TextDocument}
 */
highlights.Painter.prototype.document;

/**
 * @protected
 * @param {!highlights.Token} token
 */
highlights.Painter.prototype.paintToken = function(token) {};

/**
 * @public
 * @param {!highlights.Token} token
 */
highlights.Painter.prototype.paint = function(token) {};

/**
 * @protected
 * @param {number} start
 * @param {number} end
 * @param {string} syntax
 */
highlights.Painter.prototype.setSyntax = function(start, end, syntax) {};

/**
 * @protected
 * @param {!highlights.Token} token
 * @return {string}
 */
highlights.Painter.prototype.textOf = function(token) {};

/**
 * @public
 * @constructor
 * @param {!TextDocument} document For debugging.
 * @param {number} start
 * @param {number} end
 * @param {string} syntax
 */
highlights.Token = function(document, start, end, syntax) {};

/** @public @return {number} */
highlights.Token.prototype.end;

/** @public @return {number} */
highlights.Token.prototype.length;

/** @public @return {number} */
highlights.Token.prototype.start;

/** @public @return {string} */
highlights.Token.prototype.syntax;

/**
 * @interface
 */
highlights.TokenStateMachine = function() {};

/**
 * @param {number} state
 * @return {boolean}
 */
highlights.TokenStateMachine.prototype.isAcceptable = function(state) {};

/**
 * @param {number} state
 */
highlights.TokenStateMachine.prototype.resetTo = function(state) {};

/**
 * @param {number} state
 * @return {string}
 */
highlights.TokenStateMachine.prototype.syntaxOf = function(state) {};

/**
 * @param {number} state
 * @return {number}
 */
highlights.TokenStateMachine.prototype.updateState = function(state) {};
