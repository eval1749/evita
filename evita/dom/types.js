// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/** @enum {string} */
var Alter = {
  EXTEND: 'extend',
  MOVE: 'move'
};

/** @enum{string} */
var Direction = {
  BACKWARD: 'backward',
  FORWARD: 'forward',
  LEFT: 'left',
  RIGHT: 'right'
};

/** @enum{string} */
var NewlineMode = {
  CR: 'cr',
  CRLF: 'crlf',
  LF: 'lf',
  UNKNOWN: 'unknown'
};

/**
 * Also called granularity.
 * @enum{string}
 */
var Unit = {
  CHAR: 'char',
  DOCUMENT: 'document',
  LINE: 'line',
  PAGE: 'page',
  PARAGRAPH: 'paragraph',
  SCREEN: 'screen',
  SENTENCE: 'sentence',
  WINDOW: 'window',
  WORD: 'word'
};

/**
 * @param {Object} fields Enum type.
 * @param {string} message An error message.
 * @constructor
 */
function EnumError(fields, message) {
  TypeError.call(this, message);
  this.fields = fields;
  // TODO(yosi) This is work around of V8 bug. We should not have
  // |EnumError.message| here.
  this.message = message;
}

EnumError.prototype = Object.create(TypeError.prototype, {
  fields: {
    configurable: true,
    enumerable: true,
    value: [],
    writable: true
  }, // fields
  toString: {
    value: function() {
      return this.message + ' accepts only ' + this.fields + '.';
    },
    configurable: true,
    enumerable: true
  } // toString
});
