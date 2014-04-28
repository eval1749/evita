/**
 * @constructor
 * @param {!Document} document
 */
function Lexer(document) {}

/**
 * @type {{
 *  end: number,
 *  next_: ?Object,
 *  previous_: ?Object,
 *  state_: number,
 *  start: number,
 *  type: string
 * }}
 */
Lexer.Token;

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
 * @type {!MutationObserver}
 */
Lexer.prototype.mutationObserver_;

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
 * @type {string}
 */
Lexer.prototype.syntax;

/**
 * @param {number} hint
 * @return {number}
 */
Lexer.prototype.adjustScanOffset = function(hint) {};

/**
 */
Lexer.prototype.detach = function() {};

/**
 */
Lexer.prototype.didChangeScanOffset = function() {};

/**
 * @param {number} hint
 * @return {number}
 */
Lexer.prototype.doColor = function(hint) {};

/**
 * @return {string}
 */
Lexer.prototype.finishToken = function() {};

/**
 * @param {number} state
 * @param {string} syntax
 */
Lexer.prototype.startToken = function(state, syntax) {};
