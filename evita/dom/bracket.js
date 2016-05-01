// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('Bracket');

/** @enum {number} */
Bracket.Type = {
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
Bracket.Detail = function(type, pair) {
  this.type = type;
  this.pair = pair;
};

/** @const @type {Object.<number, !Bracket.Detail>} */
Bracket.DATA = {};

Bracket.DATA[Unicode.LEFT_PARENTHESIS] =
    new Bracket.Detail(Bracket.Type.LEFT, Unicode.RIGHT_PARENTHESIS);
Bracket.DATA[Unicode.RIGHT_PARENTHESIS] =
    new Bracket.Detail(Bracket.Type.RIGHT, Unicode.LEFT_PARENTHESIS);
Bracket.DATA[Unicode.LEFT_SQUARE_BRACKET] =
    new Bracket.Detail(Bracket.Type.LEFT, Unicode.RIGHT_SQUARE_BRACKET);
Bracket.DATA[Unicode.RIGHT_SQUARE_BRACKET] =
    new Bracket.Detail(Bracket.Type.RIGHT, Unicode.LEFT_SQUARE_BRACKET);
Bracket.DATA[Unicode.LEFT_CURLY_BRACKET] =
    new Bracket.Detail(Bracket.Type.LEFT, Unicode.RIGHT_CURLY_BRACKET);
Bracket.DATA[Unicode.RIGHT_CURLY_BRACKET] =
    new Bracket.Detail(Bracket.Type.RIGHT, Unicode.LEFT_CURLY_BRACKET);
Bracket.DATA[Unicode.REVERSE_SOLIDUS] =
    new Bracket.Detail(Bracket.Type.ESCAPE, 0x00);
