// Copyright (c) 2014 Project Vogue. All rights reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file
'use strict';

(function() {
  /** @const @type {Map.<string, string>} */
  var WORD_CLASS_MAP = (function() {
    /** @param {string} name */
    function wordClass(name) {
      var word_class = name.charAt(0);
      if (word_class === 'L' || word_class === 'N')
        return 'w';
      if (word_class === 'S')
        return 'P';
      return word_class;
    }
    var map = new Map();
    Unicode.CATEGORY_SHORT_NAMES.forEach(function(name) {
      map.set(name, wordClass(name));
    });
    return map;
  })();

  /** @enum{string} */
  var WordClass = {
    BLANK: 'Z',
    WORD: 'w'
  };

  function throwInvalidUnit(unit) {
    throw 'Invalid unit: ' + unit;
  }

  function throwNYI(name, unit) {
    throw 'NYI Range.prototype.' + name + ' Unit.' + unit;
  }

  function throwUnsupportedUnit(name, unit) {
    throw 'Range.prototype.' + name + ' does not support Unit.' + unit;
  }

  /**
   * @param {number} char_code
   * @return {WordClass|null}
   *
   * Note: We should not treat TAB and LF as blank. If we do so,
   * |Ctrl+Backspace| at start of line, removes newline at end of the previous
   * line
   */
  function wordClassOf(char_code) {
    return WORD_CLASS_MAP.get(Unicode.UCD[char_code].category);
  }

  /**
   * @param {!Document} document
   * @param {number} position
   * @return {WordClass|null}
   */
  function wordClassAt(document, position) {
    return wordClassOf(document.charCodeAt_(position));
  }

  /**
   * @param {!Document} document
   * @param {number} position
   * @return {WordClass|null}
   */
  function wordClassBefore(document, position) {
    return wordClassOf(document.charCodeAt_(position - 1));
  }

  Object.defineProperties(Document.prototype, {
    fileName: {value: '', writable: true},
    keymap: {get: /** @return {!Map} */function() {
      if (!this.keymap_)
        this.keymap_ = new Map();
      return this.keymap_;
    }},
    keymap_: {value: null, writable: true},
    lastWriteTime: {value: new Date(0), writable: true},
    mode: {
      get: /** @return {?Mode} */ function() { return this.mode_; },
      set: /** @param {?Mode} mode */ function(mode) {
        if (this.mode_)
          this.mode_.detach()
        this.mode_ = mode;
        if (mode)
          mode.attach(this);
      }
    },
    mode_: {value: null, writable: true},
    newline: {value: 0, writable: true},
    properties: {get: /** @return {!Map} */function() {
      if (!this.properties_)
        this.properties_ = new Map();
      return this.properties_;
    }},
    properties_: {value: null, writable: true},
    state: {value: 0, writable: true}
  });

  /**
   * @this {!Document}
   * @param {string} key_combination
   * @param {Object} command
   */
  Document.prototype.bindKey = function (key_combination, command) {
    var key_code = Editor.parseKeyCombination(key_combination);
    this.keymap.set(key_code, command);
  };

  /**
   * @param {Unit} unit
   * @param {number} position
   * @return {number} new position
   */
  Document.prototype.computeEndOf_ = function(unit, position) {
    var document = this;
    switch (unit) {
      case Unit.CHARACTER:
        throwUnsupportedUnit('endOf', 'CHARACTER');
      case Unit.DOCUMENT:
        return document.length;
      case Unit.LINE:
        while (position < document.length) {
          if (document.charCodeAt_(position) === Unicode.LF)
            return position;
          ++position;
        }
        return position;
      case Unit.PAGE:
        throwUnsupportedUnit('endOf', 'PAGE');
      case Unit.PARAGRAPH:
        throwUnsupportedUnit('endOf', 'PARAGRAPH');
      case Unit.SCREEN:
        throwUnsupportedUnit('endOf', 'SCREEN');
      case Unit.SENTENCE:
        throwNYI('startof', 'SENTENCE');
      case Unit.WINDOW:
        throwUnsupportedUnit('endOf', 'WINDOW');
      case Unit.WORD: {
        if (position === document.length)
          return position;
        var word_class = wordClassAt(document, position);
        if (word_class === WordClass.BLANK) {
          if (position &&
              wordClassBefore(document, position) != WordClass.BLANK) {
            // We're already at end of word
            return position;
          }
          // Skil blanks
          while (position < document.length) {
            word_class = wordClassAt(document, position);
            if (word_class != WordClass.BLANK)
              break;
            ++position;
          }
        }
        while (position < document.length) {
          if (wordClassAt(document, position) != word_class)
            break;
          ++position;
        }
        return position;
      }
      default:
        throwInvalidUnit(unit);
        return 0;
    }
  };

  /**
   * @this {!Document}
   * @param {Unit} unit
   * @param {number} count
   * @param {number} position
   * @return {number}
   *
   * Word Motion:
   *                      forward             backward
   *  th|is is a word.    this |is a word.    |this is a word
   *  this |is a word.    this is| a word.    |this is a word
   */
  Document.prototype.computeMotion_ = function(unit, count, position) {
    /** @type {!Document} */
    const document = this;
    switch (unit) {
      case Unit.BRACKET: {
        const textPosition = new TextPosition(this, position);
        textPosition.move(Unit.BRACKET, count);
        return textPosition.offset;
      }
      case Unit.CHARACTER:
        if (count > 0)
          return Math.min(document.length, position + count);
        if (count < 0)
          return Math.max(0, position + count);
        return position;
      case Unit.LINE:
        if (count > 0) {
          for (var k = 0; k < count; ++k) {
            position = document.computeEndOf_(unit, position);
            if (position === document.length)
              return position;
            ++position;
          }
          return position;
        }
        if (count < 0) {
          for (var k = count; k < 0; ++k) {
            position = document.computeStartOf_(unit, position);
            if (!position)
              break;
          }
          return position;
        }
        return position;
      case Unit.WORD:
        if (count > 0) {
          if (position === document.length)
            return position;
          for (var k = 0; k < count; ++k) {
            var word_class = wordClassAt(document, position);
            for (;;) {
              ++position;
              if (position === document.length)
                break;
              var word_class2 = wordClassAt(document, position);
              if (word_class === word_class2)
                continue;
              while (word_class2 === WordClass.BLANK) {
                ++position;
                if (position === document.length)
                  return position;
                word_class2 = wordClassAt(document, position);
              }
              break;
            }
          }
          return position;
        }

        if (count < 0) {
          if (!position)
            return position;
          for (var k = count; k < 0; ++k) {
            --position;
            var word_class = wordClassAt(document, position);
            for (;;) {
              if (!position)
                return position;
              --position;
              var word_class2 = wordClassAt(document, position);
              if (word_class === word_class2)
                continue;
              if (word_class === WordClass.BLANK) {
                while (word_class2 === WordClass.BLANK) {
                  if (!position)
                    return position;
                  --position;
                  word_class2 = wordClassAt(document, position);
                }
              }
              ++position;
              break;
            }
          }
        }
        return position;
      default:
        throw TypeError('Invalid unit: ' + unit);
    }
  };

  /**
   * @param {Unit} unit
   * @param {number} position
   * @return {number} new position
   */
  Document.prototype.computeStartOf_ = function(unit, position) {
    var document = this;
    switch (unit) {
      case Unit.CHARACTER:
        throwUnsupportedUnit('startOf', 'CHARACTER');
      case Unit.DOCUMENT:
        return 0;
      case Unit.LINE:
        while (position > 0) {
          --position;
          if (document.charCodeAt_(position) === Unicode.LF)
            return position + 1;
        }
        return position;
      case Unit.PAGE:
        throwUnsupportedUnit('startOf', 'PAGE');
      case Unit.PARAGRAPH:
        throwUnsupportedUnit('startOf', 'PARAGRAPH');
      case Unit.SCREEN:
        throwUnsupportedUnit('startOf', 'SCREEN');
      case Unit.SENTENCE:
        throwNYI('startof', 'SENTENCE');
      case Unit.WINDOW:
        throwUnsupportedUnit('startof', 'WINDOW');
      case Unit.WORD: {
        if (!position)
          return position;
        var word_class = position === document.length ?
            WordClass.BLANK : wordClassAt(document, position);
        // Find character class of word
        while (word_class === WordClass.BLANK) {
          --position;
          if (!position)
            return position;
          word_class = wordClassAt(document, position);
        }
        // Skip word
        while (position && wordClassAt(document, position - 1) === word_class)
          --position;
        return position;
      }
      default:
        throwInvalidUnit(unit);
        return 0;
    }
  }

  /**
   * @this {!Document}
   * @param {string} char_set
   * @param {number} count
   * @param {number} start
   * @return {number}
   */
  function computeWhile_(char_set, count, start) {
    var char_code_set = new Set();
    for (var i = 0; i < char_set.length; ++i)
      char_code_set.add(char_set.charCodeAt(i));

    if (count > 0) {
      var end = Math.min(start + count, this.length);
      for (var position = start; position < end; ++position) {
        var char_code = this.charCodeAt_(position);
        if (!char_code_set.has(char_code))
          return position;
      }
      return end;
    }

    var end = Math.max(start + count, 0);
    for (var position = start; position > end; --position) {
      var char_code = this.charCodeAt_(position - 1);
      if (!char_code_set.has(char_code))
        return position;
    }
    return end;
  }

  /**
   * @this {!Document}
   * @param {number} hint
   */
  function doColor_(hint) {
    if (!this.mode)
      return;
    this.mode.doColor(this, hint);
  }

  /**
   * @this {!Document}
   *
   * Returns generator object which returns line, including newline character,
   * in a document.
   */
  function* lines() {
    var range = new Range(this);
    while (range.start < this.length) {
      range.moveEnd(Unit.LINE);
      yield range.text;
      range.collapseTo(range.end);
    }
  }

  /**
   * @this {!Document}
   * @return {Array.<!DocumentWindow>}
   */
  function listWindows() {
    var windows = [];
    for (var editorWindow of EditorWindow.list) {
      for (var window of editorWindow.children) {
        if ((window instanceof DocumentWindow) && window.document === this)
          windows.push(window);
      }
    }
    return windows;
  };

  /**
   * @this {!Document}
   * @param {string} name
   * @param {function()} callback
   * @param {!Object=} opt_receiver
   */
  function undoGroup(name, callback, opt_receiver) {
    var document = this;
    var receiver = opt_receiver === undefined ? document : opt_receiver;
    try {
      document.startUndoGroup_(name);
      callback.call(receiver);
    } finally {
      document.endUndoGroup_(name);
    }
  }

  Object.defineProperties(Document.prototype, {
    computeWhile_: {value: computeWhile_},
    doColor_: {value: doColor_},
    lines: {get: lines},
    listWindows: {value: listWindows},
    undoGroup: {value: undoGroup}
  });
})();
