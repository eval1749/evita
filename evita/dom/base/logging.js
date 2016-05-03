// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

const kEscapes = '\nn\rr\tt\'\'""\\\\';

/**
 * @param {string} text
 * @return {string}
 */
function asStringLiteral(text) {
  /** @const @type {!Array<string>} */
  const result = [];
  for (let index = 0; index < text.length; ++index) {
    /** @const @type {number} */
    const charCode = text.charCodeAt(index);
    const charText = text[index];
    const escapeIndex = kEscapes.indexOf(charText);
    if (escapeIndex % 2 == 0) {
      result.push(`\\${kEscapes[escapeIndex + 1]}`);
      continue;
    }
    if (charCode < 0x20) {
      result.push(`\\u{${charCode.toString(16)}}`);
      continue;
    }
    result.push(charText);
  }
  return `'${result.join('')}'`;
}
base.asStringLiteral = asStringLiteral;
});
