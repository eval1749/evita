// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var Bracket = {};

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
Bracket.Detail = function(type, pair) {};

/** @type{number} */ Bracket.Detail.prototype.pair;
/** @type{!Bracket.Type} */ Bracket.Detail.prototype.type;

/** @const @type {Object.<number, !Bracket.Detail>} */
Bracket.DATA = {};
