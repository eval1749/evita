// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

  Object.defineProperty(Document.prototype, 'lines', {
    /**
     * @param {!Document} document
     * @return {!Generator.<string>}
     *
     * Returns generator object which returns line, including newline character,
     * in a document.
     */
    get: function*() {
      var range = new Range(this);
      while (range.start < this.length) {
        range.moveEnd(Unit.LINE);
        yield range.text;
        range.collapseTo(range.end);
      }
      return '';
    }
  });


function NewRegExp(source, options) {
  return new RegExp(source, options)
}
