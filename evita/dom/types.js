// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/** @enum {string} */
var Alter = {
  EXTEND: 'extend',
  MOVE: 'move'
};

/** @enum{number} */
var DialogItemId = {
  ABORT: 3,
  CANCEL: 2,
  CONTINUE: 11,
  IGNORE: 5,
  NO: 7,
  OK: 1,
  RETRY: 4,
  TRYAGAIN: 10,
  YES: 6
};

/** @enum{string} */
var Direction = {
  BACKWARD: 'backward',
  FORWARD: 'forward',
  LEFT: 'left',
  RIGHT: 'right'
};

/** @enum{number} */
var MessageBox = {
  ABORTRETRYIGNORE: 0x2,
  CANCELTRYCONTINUE: 0x6,
  HELP: 0x4000,
  OK: 0x000,
  OKCANCEL: 0x0001,
  RETRYCANCEL: 0x0005,
  YESNO: 0x0004,
  YESNOCANCEL: 0x0003,
  ICONEXCLAMATION: 0x0030,
  ICONWARNING: 0x0030,
  ICONINFORMATION: 0x0040,
  ICONASTERISK: 0x0040,
  ICONQUESTION: 0x0020,
  ICONSTOP: 0x0010,
  ICONERROR: 0x0010,
  ICONHAND: 0x0010,
  DEFBUTTON1: 0x0000,
  DEFBUTTON2: 0x0100,
  DEFBUTTON3: 0x0200,
  DEFBUTTON4: 0x0400,
  APPLMODAL: 0x0000,
  SYSTEMMODAL: 0x1000,
  TASKMODAL: 0x2000,
  DEFAULT_DESKTOP_ONLY: 0x20000,
  RIGHT: 0x80000,
  RTLREADING: 0x100000,
  SETFOREGROUND: 0x10000,
  TOPMOST: 0x40000,
  SERVICE_NOTIFICATION: 0x200000
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
