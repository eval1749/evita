// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/** @enum {string} */
var Alter = {
  EXTEND: 'EXTEND',
  MOVE: 'MOVE'
};

/** @enum {number} */
var Count = {
  BACKWARD: -1 << 28,
  FORWARD: 1 << 28,
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
  BACKWARD: 'BACKWARD',
  FORWARD: 'FORWARD',
  LEFT: 'LEFT',
  RIGHT: 'RIGHT'
};

/**
 * @enum {!Symbol}
 */
Document.Obsolete = {
  CHECKING: Symbol('CHECKING'),
  IGNORE: Symbol('IGNORE'),
  NO: Symbol('NO'),
  UNKNOWN: Symbol('UNKNOWN'),
  YES: Symbol('YES')
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

/** @enum{number} */
var Newline = {
  UNKNOWN: 0,
  LF: 1,
  CRLF: 3,
};

/** @enum{number} */
var TableViewRowState = {
  FOCUSED: 0x0001,
  SELECTED: 0x0002,
  CUT: 0x0004,
  DROPHILITED: 0x0008,
  GLOW: 0x0010,
  ACTIVATING: 0x0020
};

/** @enum{number} */
var TextWindowComputeMethod = {
  END_OF_WINDOW: 0,
  END_OF_WINDOW_LINE: 1,
  MAP_POINT_TO_POSITION: 2,
  MAP_POSITION_TO_POINT: 3,
  MOVE_SCREEN: 4,
  MOVE_WINDOW: 5,
  MOVE_WINDOW_LINE: 6,
  START_OF_WINDOW: 7,
  START_OF_WINDOW_LINE: 8,
};

/**
 * Also called granularity.
 * @enum{string}
 */
var Unit = {
  BRACKET: 'BRACKET',
  CHARACTER: 'CHARACTER',
  DOCUMENT: 'DOCUMENT',
  LINE: 'LINE',
  PAGE: 'PAGE',
  PARAGRAPH: 'PARAGRAPH',
  SCREEN: 'SCREEN',
  SENTENCE: 'SENTENCE',
  WINDOW: 'WINDOW',
  WINDOW_LINE: 'WINDOW_LINE',
  WORD: 'WORD'
};
