// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @struct { name: string, start: number, end: number }
 */
Editor.RegExp.Match = (function() {
  function Match() {
  }
  return Match;
})();

/**
 * @param {string|!Document|!Range} target
 * @param {number=} opt_start
 * @param {number=} opt_end
 * @return {?Array.<string>}
 */
global.Editor.RegExp.prototype.exec = function(target, opt_start, opt_end) {
  function execOnDocument(regexp, document, start, end) {
    var matches = document.match_(regexp, start, end);
    if (!matches)
      return null;
    var range = new Range(document);
    return matches.map(function(match) {
      // TODO(yosi) We should use |Document.prototype.substring|.
      range.collapseTo(match.start);
      range.end = match.end;
      return range.text;
    });
  }
  if (target instanceof Document) {
    var document = /** @type {!Document} */(target);
    var start = arguments.length >= 2 ? /** @type {number} */(opt_start) : 0;
    var end = arguments.length >= 3 ? /** @type {number} */(opt_end) :
                                      document.length;
    return execOnDocument(this, document, start, end);
  }

  if (target instanceof Range) {
    var range = /** @type {!Range} */(target);
    return execOnDocument(this, range.document, range.start, range.end);
  }

  var string = target.toString();
  var matches = this.exec_(string);
  if (!matches)
    return null;
  var strings = [];
  return matches.map(function(match) {
    return string.substring(match.start, match.end);
  });
};
