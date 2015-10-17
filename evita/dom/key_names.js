// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/** @const @type{Array.<string>} */
global.VKEY_NAMES = [
  '00', // 0x00
  'LeftButton', // 0x01
  'RightButton', // 0x02
  'Cancel', // 0x03
  'MiddleButton', // 0x04
  'XButton1', // 0x05
  'XButton2', // 0x06
  '07', // 0x07
  'Backspace', // 0x08
  'Tab', // 0x09
  '0A', // 0x0A
  '0B', // 0x0B
  'Clear', // 0x0C
  'Enter', // 0x0D
  '0E', // 0x0E
  '0F', // 0x0F

  'Shift', // 0x10
  'Control', // 0x11
  'Menu', // 0x12
  'Pause', // 0x13
  'Capital', // 0x14
  'Kana', // 0x15
  '016', // 0x16
  'Junja', // 0x17
  'Final', // 0x18
  'Kanji', // 0x19
  '01A', // 0x1A
  'Escape', // 0x1B
  'Convert', // 0x1C
  'NonConvert', // 0x1D
  'Accept', // 0x1E
  'ModeChange', // 0x1F

  'Space', // 0x20
  'PageUp', // 0x21
  'PageDown', // 0x22
  'End', // 0x23
  'Home', // 0x24
  'ArrowLeft', // 0x25
  'ArrowUp', // 0x26
  'ArrowRight', // 0x27
  'ArrowDown', // 0x28
  'Select', // 0x29
  'Print', // 0x2A
  'Execute', // 0x2B
  'Snapshot', // 0x2C
  'Insert', // 0x2D
  'Delete', // 0x2E
  'Help', // 0x2F

   '30', '31', '32', '33', '34', '35', '36', '37',
   '38', '39', '3A', '3B', '3C', '3D', '3E', '3F',
   '40', '41', '42', '43', '44', '45', '46', '47',
   '48', '49', '4A', '4B', '4C', '4D', '4E', '4F',
   '50', '51', '52', '53', '54', '55', '56', '57',
   '58', '59', '5A',

   'LeftWin', // 0x5B
   'RightWin', // 0x5C
   'Apps', // 0x5D
   '5E', // 0x5E
   'Sleep', // 0x5F

   'NumPad0', 'NumPad1', 'NumPad2', 'NumPad3', // 0x60
   'NumPad4', 'NumPad5', 'NumPad6', 'NumPad7', // 0x64
   'NumPad8', 'NumPad9',
   'Multiply', // 0x6A
   'Add', // 0x6B
   'Separator', // 0x6C
   'Subtract', // 0x6D
   'Decimal', // 0x6E
   'Divide', // 0x6F

   'F1', 'F2', 'F3', 'f4', 'F5', 'F6', 'F7', 'F8', // 0x70
   'F9', 'F10', 'F11', 'f12', 'F13', 'F14', 'F15', 'F16', // 0x978
   'F17', 'F18', 'F19', 'F20', 'F21', 'F22', 'F23', 'F24', // 0x80
   '88', '89', '8A', '8B', '8C', '8D', '8E', '8F',

   'NumLock', // 0x90
   'ScrLk', // 0x91
   'OemFjJisho', // 0x92
   'OemFjMasshou', // 0x93
   'OemFjTouroku', // 0x94
   'OemFjLeftOya', // 0x95
   'OemFjRightOya', // 0x96
   '97', // 0x97
   '98', '99', '9A', '9B', '9C', '9D', '9E', '9F',

  // VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
  // Used only as parameters to GetAsyncKeyState() and GetKeyState().
  // No other API or message will distinguish left and right keys in this
  // way.
  'LeftShift', // 0xA0
  'RightShift', // 0xA1
  'LeftControl', // 0xA2
  'RightControl', // 0xA3
  'LeftMenu', // 0xA4
  'RightMenu', // 0xA5
  'BROWSER_BACK',      // 0xA6
  'BROWSER_FORWARD',   // 0xA7
  'BROWSER_REFRESH',   // 0xA8
  'BROWSER_STOP',      // 0xA9
  'BROWSER_SEARCH',    // 0xAA
  'BROWSER_FAVORITES', // 0xAB
  'BROWSER_HOME',      // 0xAC
  'VOLUME_MUTE',       // 0xAD
  'VOLUME_DOWN',       // 0xAE
  'VOLUME_UP',         // 0xAF

  'MEDIA_NEXT_TRACK',  // 0xB0
  'MEDIA_PREV_TRACK',  // 0xB1
  'MEDIA_STOP',        // 0xB2
  'MEDIA_PLAY_PAUSE',  // 0xB3
  'LAUNCH_MAIL',       // 0xB4
  'LAUNCH_MEDIA_SELECT', // 0xB5
  'LAUNCH_APP1',       // 0xB6
  'LAUNCH_APP2',       // 0xB7
  'B8', 'B9',
  'OEM_1',         // 0xBA   // ';:' for US
  'OEM_PLUS',      // 0xBB   // '+' any country
  'OEM_COMMA',     // 0xBC   // ',' any country
  'OEM_MINUS',     // 0xBD   // '-' any country
  'OEM_PERIOD',    // 0xBE   // '.' any country
  'OEM_2',         // 0xBF   // '/?' for US

  'OEM_3',         // 0xC0   // '`~' for US
         'C1', 'C2', 'C3', 'C4', 'C5', 'C6', 'C7',
  'C8', 'C9', 'CA', 'CB', 'CC', 'CD', 'CE', 'CF',

  'D0', 'D1', 'D2', 'D3', 'D4', 'D5', 'D6', 'D7',
  'D8', 'D9', 'DA',
  'OEM_4', // 0xDB  //  '[{' for US
  'OEM_5', // 0xDC  //  '\|' for US
  'OEM_6', // 0xDD  //  ']}' for US
  'OEM_7', // 0xDE  //  '''' for US
  'OEM_8', // 0xDF

  'E0',
  'OEM_AX',    // 0xE1  //  'AX' key on Japanese AX kbd
  'OEM_102',   // 0xE2  //  '<>' or '\|' on RT 102-key kbd.
  'ICO_HELP',  // 0xE3  //  Help key on ICO
  'ICO_00',    // 0xE4  //  00 key on ICO
  'PROCESSKEY', // 0xE5
  'ICO_CLEAR', // 0xE6
  'PACKET',    // 0xE7
  'E8',
  // Nokia/Ericsson definitions
  'OEM_RESET', // 0xE9
  'OEM_JUMP',  // 0xEA
  'OEM_PA1',   // 0xEB
  'OEM_PA2',   // 0xEC
  'OEM_PA3',   // 0xED
  'OEM_WSCTRL',// 0xEE
  'OEM_CUSEL', // 0xEF

  'OEM_ATTN',  // 0xF0
  'OEM_FINISH',// 0xF1
  'OEM_COPY',  // 0xF2
  'OEM_AUTO',  // 0xF3
  'OEM_ENLW',  // 0xF4
  'OEM_BACKTAB',//0xF5
  'ATTN',      // 0xF6
  'CRSEL',     // 0xF7
  'EXSEL',     // 0xF8
  'EREOF',     // 0xF9
  'PLAY',      // 0xFA
  'ZOOM',      // 0xFB
  'NONAME',    // 0xFC
  'PA1',       // 0xFD
  'OEM_CLEAR', // 0xFE
  'FF'
];

/** @const @type {!Array.<string>} */
global.KEY_NAMES = (function() {
  /** @const @type {Array.<string>} */
  var names = new Array(256);
  /**
   * @param{number} start
   * @param{number} end
   */
  function populate(start, end) {
    for (var key_code = start; key_code <= end; ++key_code) {
      names[key_code] = String.fromCharCode(key_code);
    }
  }
  // TODO(eval1749): We should get supported key code values for current keyboard
  // from |::MapVirtualKey(vkey, MAPVK_VK_TOCHAR)|.
  populate(0x20, 0x60);
  populate(0x7B, 0x7E);
  VKEY_NAMES.forEach(function(name, index) {
    names[index + 0x100] = name;
  });
  return names;
})();

/** @const @type {!Object.<string, number>} */
global.KEY_CODE_MAP = (function() {
  var map = {};
  KEY_NAMES.forEach(function(name, index) {
    if (name) {
      map[name.toLowerCase()] = index;
    }
  });
  return map;
})();
