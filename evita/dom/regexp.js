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
 * @param {*} target
 * @return {?Array.<string>}
 */
global.Editor.RegExp.prototype.exec = function(target) {
  if (target instanceof Range) {
    var range = /** @type {!Range} */(target);
    return range.match(this);
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
