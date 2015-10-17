// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @type {!TextFieldControl} */
TextFieldBuffer.prototype.value_;

/**
 * @constructor
 * @param {!TextFieldControl} control
 */
global.TextFieldBuffer = function(control) {
  this.control_ = control;
};

(function() {

/**
 * @param {!TextFieldControl} control
 * @param {!function(!TextFieldRange)} callback
 */
// TODO(eval1749): Until |WeakSet| supports |forEach| method, we use
// to sue |WeakMap|.
function rangeSetForEach(control, callback) {
  // Relocate ranges
  var rangeSet = this.ranges_;
  var numRanges = rangeSet['size_'];
  var ranges = [];
  for (var key = 0; key < numRanges; ++key) {
    ranges.push(rangeSet.get(key));
  }
  rangeSet.clear();
  rangeSet['size_'] = ranges.length;
  ranges.forEach(function(range, index) {
    rangeSet.add(index, range);
    callback(range);
  });
}

/**
 * @param {!TextFieldBuffer} buffer
 * @param {number} offset
 */
function validateOffset(buffer, offset) {
  if (offset >= 0 && offset <= buffer.length)
    return;
  throw new RangeError('Bad offset ' + offset);
}

/**
 * @this {!TextFieldBuffer}
 * @param {number} start
 * @param {number} end
 */
function validateRange(buffer, start, end) {
  validateOffset(buffer, start);
  validateOffset(buffer, end);
  if (start > end)
    throw new RangeError('Bad offsets ' + start + ', ' + end);
}

Object.defineProperties(TextFieldBuffer.prototype, {
  delete: {
    value:
      /**
       * @this {!TextFieldBuffer}
       * @param {number} start
       * @param {number} end
       */
     function(start, end) {
       validateRange(this, start, end);
       var length = end - start;
       if (!length)
        return;
       this.replace(start, end, '');
     }
  },
  insertBefore: {
    value:
      /**
       * @this {!TextFieldBuffer}
       * @param {number} offset
       * @param {string} new_text
       */
     function(offset, new_text) {
       validateOffset(this, offset);
        if (new_text == '')
          return;
        this.replace(offset, offset, new_text);
     },
  },
  length: {
    get:
      /**
       * @this {!TextFieldBuffer}
       */
      function() {
        return this.text.length;
      }
  },
  replace: {
    value:
      /**
       * @this {!TextFieldBuffer}
       * @param {number} start
       * @param {number} end
       * @param {string} string
       */
     function(start, end, string) {
       var text = this.text;
       // TODO(eval1749): We should record old value for undo/redo.
       this.control_.value_ = text.substr(0, start) + string + text.substr(end);
       var delta = string.length - (end - start);
       rangeSetForEach(this, function(range) {
         if (range.start_ >= start)
           range.start_ = Math.max(start, range.start + delta);
         if (range.end_ >= start)
            range.start_ = Math.max(start, range.end + delta);
        });
     },
  },
  substring: {
    value:
      /**
       * @this {!TextFieldBuffer}
       * @param {number} start
       * @param {number} end
       * @return {string}
       */
     function(start, end) {
       validateRange(this, start, end);
       return this.text.substring(start, end);
     }
  },
  text: {
      /**
       * @this {!TextFieldBuffer}
       * @return {string}
       */
     function() {
       return this.control_.value_;
     }
  }
});
})();
