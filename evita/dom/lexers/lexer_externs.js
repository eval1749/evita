/**
 * @constructor
 * @param {Iterable.<string>} keywords
 * @param {!Document} document
 */
function Lexer(keywords, document) {}

/**
 * @constructor
 * @param {number} state
 * @param {number} start
 */
Lexer.Token = function(state, start) {};

/** @type {number} */
Lexer.Token.prototype.end;

/** @type {number} */
Lexer.Token.prototype.start;

/** @type {number} */
Lexer.Token.prototype.state;

/** @type {string} */
Lexer.Token.prototype.type;

/**
 * @type {number}
 */
Lexer.prototype.changeOffset;

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
 * @type {number}
 */
Lexer.prototype.state;

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
Lexer.prototype.extendToken = function() {};

/**
 * @param {number} next_state
 * @return {!Lexer.Token}
 */
Lexer.prototype.finishToken = function(next_state) {};

/**
 * @param {number} state
 * @return {!Lexer.Token}
 */
Lexer.prototype.finishTokenAs = function(state) {};

/**
 * @param {number} state
 */
Lexer.prototype.startToken = function(state) {};
