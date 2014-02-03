// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Note: When I put |Bracket| into IIFE. Closure compiler can't find
// |Bracket.Type| and  |Bracket.Data|.
global.Bracket = {};

/** @enum {number} */
global.Bracket.Type = {
  NONE: 0,
  ESCAPE: 2,
  LEFT: -1,
  RIGHT: 1,
};

/**
 * @constructor
 * @struct
 * @param {!Bracket.Type} type
 * @param {number} pair
 */
global.Bracket.Detail = function(type, pair) {
  this.type = type;
  this.pair = pair;
}

/** @const @type {Object.<number, !Bracket.Detail>} */
global.Bracket.DATA = {};

global.Bracket.DATA[Unicode.LEFT_PARENTHESIS] = new Bracket.Detail(
    Bracket.Type.LEFT, Unicode.RIGHT_PARENTHESIS);
global.Bracket.DATA[Unicode.RIGHT_PARENTHESIS] = new Bracket.Detail(
    Bracket.Type.RIGHT, Unicode.LEFT_PARENTHESIS);
global.Bracket.DATA[Unicode.LEFT_SQUARE_BRACKET] = new Bracket.Detail(
    Bracket.Type.LEFT, Unicode.RIGHT_SQUARE_BRACKET);
global.Bracket.DATA[Unicode.RIGHT_SQUARE_BRACKET] = new Bracket.Detail(
    Bracket.Type.RIGHT, Unicode.LEFT_SQUARE_BRACKET);
global.Bracket.DATA[Unicode.LEFT_CURLY_BRACKET] = new Bracket.Detail(
    Bracket.Type.LEFT, Unicode.RIGHT_CURLY_BRACKET);
global.Bracket.DATA[Unicode.RIGHT_CURLY_BRACKET] = new Bracket.Detail(
    Bracket.Type.RIGHT, Unicode.LEFT_CURLY_BRACKET);
global.Bracket.DATA[Unicode.REVERSE_SOLIDUS] = new Bracket.Detail(
    Bracket.Type.ESCAPE, 0x00);
