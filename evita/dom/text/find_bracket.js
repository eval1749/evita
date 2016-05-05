// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

/** @const @type {!Bracket.Detail} */
const NOT_BRACKET = new Bracket.Detail(Bracket.Type.NONE, 0);

/** @const @type {number} */
const MAX_BRACKET_NESTING_LEVEL = 10;

class BracketMatchData {
  /**
   * @param {!Bracket.Detail} data
   * @param {!TextOffset} offset
   */
  constructor(data, offset) {
    this.data = data;
    this.offset = offset;
  }
}

/**
 * @param {!TextDocument} document
 * @param {number} offset
 * @return {!Bracket.Detail}
 */
function bracketDataOf(document, offset) {
  /** @const {number} */
  const charCode = document.charCodeAt(offset);
  // TODO(eval1749): We should get character syntax from mime type
  // information.
  return Bracket.DATA[charCode] || NOT_BRACKET;
}

/**
 * @param {string} syntax1
 * @param {string} syntax2
 * @return {boolean}
 */
function syntaxEquals(syntax1, syntax2) {
  if (syntax1 === syntax2)
    return true;
  return syntax1 === 'normal' || syntax2 === 'normal';
}

/**
 * @param {!TextDocument} document
 * @param {number} start
 * @param {string} bracketCharSyntax
 * @return {number}  An offset after non-escape character.
 */
function whileEscapeBackward(document, start, bracketCharSyntax) {
  /** @type {number} */
  let offset = start - 1;
  while (offset >= 0) {
    if (!syntaxEquals(document.syntaxAt(offset), bracketCharSyntax))
      break;
    if (bracketDataOf(document, offset).type !== Bracket.Type.ESCAPE)
      break;
    --offset;
  }
  return offset + 1;
}

/**
 * @param {!TextDocument} document
 * @param {number} start
 * @param {string} bracketCharSyntax
 * @return {number}  An offset before non-escape character.
 */
function whileEscapeForward(document, start, bracketCharSyntax) {
  /** @type {number} */
  let offset = start;
  while (offset < document.length) {
    if (!syntaxEquals(document.syntaxAt(offset), bracketCharSyntax))
      break;
    if (bracketDataOf(document, offset).type !== Bracket.Type.ESCAPE)
      break;
    ++offset;
  }
  return offset;
}

/**
 * @param {!TextDocument} document
 * @param {number} start
 * @return {number}
 *
 * Note: We use syntax highlighting information for preventing matching
 * parenthesis among statement, string and comment.
 * See also |findBracketForward()|.
 */
function findBracketBackward(document, start) {
  /** @type {!Array<!BracketMatchData>} */
  const bracketStack = [];
  // reset when we reached at bracket.
  /**  @type {string} */
  let bracketCharSyntax = '?';
  for (let offset = start - 1; offset >= 0; --offset) {
    /**  @const @type {!Bracket.Detail} */
    const bracket = bracketDataOf(document, offset);
    if (bracket.type === Bracket.Type.NONE)
      continue;
    if (bracketCharSyntax === '?')
      bracketCharSyntax = document.syntaxAt(offset);
    else if (!syntaxEquals(document.syntaxAt(offset), bracketCharSyntax))
      continue;

    /** @const @type {number} */
    const next = whileEscapeBackward(document, offset, bracketCharSyntax);
    if ((offset - next) % 2 == 1)
      continue;

    if (bracket.type === Bracket.Type.RIGHT) {
      if (bracketStack.length === 0 && offset !== start - 1) {
        // We found right bracket.
        //  ...|) ... ^
        // Return after bracket:
        //  ... )| ... ^
        return offset + 1;
      }
      bracketStack.push(new BracketMatchData(bracket, offset));
      offset = next;
      continue;
    }

    if (bracket.type !== Bracket.Type.LEFT) {
      offset = next;
      continue;
    }

    if (bracketStack.length === 0) {
      // We reach at left bracket before seeing right bracket:
      //  |( ... ^
      return offset;
    }

    /** @const @type {!BracketMatchData} */
    const lastBracket = bracketStack.pop();
    if (lastBracket.data.pair !== document.charCodeAt(offset)) {
      // We reach at mismatched left bracket.
      //  |( ... }^
      // Back to start
      //   ( ... }|^
      return start;
    }

    if (bracketStack.length === 0) {
      // We reach at matched left bracket.
      //  |( ... (...) ... )^
      return offset;
    }
    offset = next;
  }
  return start;
}

/**
 * @param {!TextDocument} document
 * @param {number} start
 * @return {number}
 *
 * Note: We use syntax coloring information for preventing matching
 * parenthesis among statement, string and comment.
 * See also |findBracketBackward()|.
 */
function findBracketForward(document, start) {
  /** @type {!Array<!BracketMatchData>} */
  const bracketStack = [];
  // reset when we reached at racket.
  /** @type {string} */
  let bracketCharSyntax = '?';

  for (let offset = start; offset < document.length; ++offset) {
    /**  @const @type {!Bracket.Detail} */
    const bracket = bracketDataOf(document, offset);
    if (bracket.type === Bracket.Type.NONE)
      continue;
    if (bracketCharSyntax === '?')
      bracketCharSyntax = document.syntaxAt(offset);
    else if (!syntaxEquals(document.syntaxAt(offset), bracketCharSyntax))
      continue;
    switch (bracket.type) {
      case Bracket.Type.ESCAPE: {
        /** @const @type {number} */
        const next =
            whileEscapeForward(document, offset + 1, bracketCharSyntax);
        if ((next - offset) % 2 === 0) {
          offset = next - 1;
          continue;
        }
        offset = next;
        break;
      }
      case Bracket.Type.LEFT:
        if (bracketStack.length === 0) {
          if (offset !== start) {
            // We reach left bracket.
            return offset;
          }
          bracketStack.push(new BracketMatchData(bracket, offset));
          break;
        }

        if (bracketStack.length === MAX_BRACKET_NESTING_LEVEL) {
          // We found too many left bracket.
          return offset;
        }
        bracketStack.push(new BracketMatchData(bracket, offset));
        break;
      case Bracket.Type.RIGHT:
        if (bracketStack.length === 0) {
          // We reach right bracket before seeing left bracket:
          //    ^...}
          // Move to next of right bracket:
          //    ^...}|
          return offset + 1;
        }
        /** @const @type {!BracketMatchData} */
        const lastBracket = bracketStack.pop();
        if (lastBracket.data.pair === document.charCodeAt(offset)) {
          if (bracketStack.length > 0)
            continue;
          // We reach matched right bracket:
          //    ^(...|)
          // Move to next of right bracket:
          //    ^(...)|
          return offset + 1;
        }
        // We reach nested mismatched right bracket:
        //    ^{ if (foo ... |} or { if ^(foo ... |}
        // Back to left bracket to know mismatched bracket:
        //    ^{ if |(foo ... } or { if ^(foo ... |}
        return lastBracket.offset;
    }
  }
  if (bracketStack.length === 0) {
    // There are no brackets after |start|.
    return start;
  }
  // There are unclosed left bracket:
  //    ^( ...  ( ...
  // Return before last unclosed bracket:
  //    ^( ... |( ...
  return bracketStack.pop().offset;
}

/**
 * @param {!TextDocument} document
 * @param {number} start
 * @return {number}
 */
text.findBracketBackward = findBracketBackward;

/**
 * @param {!TextDocument} document
 * @param {number} start
 * @return {number}
 */
text.findBracketForward = findBracketForward;

});
