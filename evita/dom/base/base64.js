// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'base', function($export) {
  /** @const @type {string} */
  const BASE64_CHARS =
      'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';

  /** @const @type {string} */
  const INVALID_BASE64_INPUT = 'Invalid base64 input';

  /** @const @type {!Array<number>} */
  const DIGIT_MAP = (function() {
    const map = new Array(127);
    map.fill(-1);
    for (let index = 0; index < BASE64_CHARS.length; ++index)
      map[BASE64_CHARS.charCodeAt(index)] = index;
    return map;
  })();

  /**
   * @param {number} charCode
   * @return {number}
   */
  function fromBase64Digit(charCode) {
    if (charCode >= DIGIT_MAP.length) {
      throw new Error(
          INVALID_BASE64_INPUT + ': ' + String.fromCharCode(charCode));
    }
    return DIGIT_MAP[charCode];
  }

  /**
   * @param {number} digit64
   * @return {string}
   */
  function toBase64Digit(digit64) {
    return digit64 < 0 ? '=' : BASE64_CHARS.charAt(digit64);
  }

  /**
   * @param {string} input
   * @return {!Uint8Array}
   */
  function atob(input) {
    if (input.length % 4 !== 0)
      throw new Error(INVALID_BASE64_INPUT + ': length=' + input.length);
    const equal = input.indexOf('=');
    /** @type {number} */
    const inputEnd = equal < 0 ? input.length : equal;
    /** @type {number} */
    const size = Math.floor(input.length * 3 / 4) -
        (equal > 0 ? input.length - equal : 0);
    /** @const @type {!Uint8Array} */
    const output = new Uint8Array(size);
    /** @type {number} */
    let outputIndex = 0;
    for (let index = 0; index < inputEnd; index += 4) {
      const digit0 = fromBase64Digit(input.charCodeAt(index));
      const digit1 = fromBase64Digit(input.charCodeAt(index + 1));
      output[outputIndex] = (digit0 << 2) | (digit1 >> 4);
      ++outputIndex;
      if (outputIndex == output.length)
        break;
      const digit2 = fromBase64Digit(input.charCodeAt(index + 2));
      output[outputIndex] = (digit1 << 4) | (digit2 >> 2);
      ++outputIndex;
      if (outputIndex == output.length)
        break;
      const digit3 = fromBase64Digit(input.charCodeAt(index + 3));
      output[outputIndex] = (digit2 << 6) | digit3;
      ++outputIndex;
    }
    return output;
  }

  /**
   * @param {!Uint8Array} input
   * @return {string}
   */
  function btoa(input) {
    /** @const @type {number} */
    const base64Count = Math.ceil(input.length * 8 / 6);
    /** @const @type {!Array<number>} */
    const output = new Array(Math.ceil(base64Count / 4) * 4);
    /** @type {number} */
    let outputIndex = 0;
    for (let index = 0; index < input.length; index += 3) {
      output[outputIndex] = (input[index] & 0xFC) >> 2;
      ++outputIndex;
      output[outputIndex] = (input[index] & 3) << 4;
      if (index + 1 >= input.length) {
        ++outputIndex;
        output[outputIndex] = -1;
        ++outputIndex;
        output[outputIndex] = -1;
        break;
      }
      output[outputIndex] |= (input[index + 1] & 0xF0) >> 4;
      ++outputIndex;
      output[outputIndex] = (input[index + 1] & 0x0F) << 2;
      if (index + 2 >= input.length) {
        ++outputIndex;
        output[outputIndex] = -1;
        break;
      }
      output[outputIndex] |= (input[index + 2] & 0xC0) >> 6;
      ++outputIndex;
      output[outputIndex] = input[index + 2] & 0x3F;
      ++outputIndex;
    }
    return output.map(toBase64Digit).join('');
  }

  $export({atob, btoa});
});
