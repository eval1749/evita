// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

(function() {
  /** @const @type {Map.<string, string>} */
  var WORD_CLASS_MAP = (function() {
    function wordClass(name) {
      var word_class = name.charAt(0);
      if (word_class == 'L' || word_class == 'N')
        return 'w';
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
    WORD: 'w',
  };

  function doesNotSupportUnit(name, unit) {
    throw 'Range.prototype.' + name + ' does not support Unit.' + unit;
  }

  function NYI(name, unit) {
    throw 'NYI Range.prototype.' + name + ' Unit.' + unit;
  }

  function wordClassOf(char_code) {
    return WORD_CLASS_MAP.get(Unicode.UCD[char_code].category);
  }

  /**
   * Capitalize range.
   * @this {Range}
   */
  Range.prototype.capitalize = function() {
    var text = this.text;
    for (var i = 0; i < text.length; ++i) {
      var data = Unicode.UCD[text.charCodeAt(i)];
      if (data.category == Unicode.Category.Lu ||
          data.category == Unicode.Category.Ll ||
          data.category == Unicode.Category.Lt) {
        this.text = text.substr(0, i) + text.charAt(i).toLocaleUpperCase() +
                    text.substr(i + 1).toLocaleLowerCase();
        return;
      }
    }
  };

  /**
   * Move end position of Range at end of specified unit.
   * @this {Range}
   * @param {Unit} unit.
   * @param {Alter} alter, optional default is Alter.MOVE.
   */
  Range.prototype.endOf = function(unit, alter) {
    alter = arguments.length == 1 ? Alter.MOVE : alter;
    var document = this.document;
    switch (unit) {
      case Unit.CHARACTER:
        doesNotSupportUnit('endOf', 'CHARACTER');
      case Unit.DOCUMENT:
        this.end = document.length;
        break;
      case Unit.LINE:
        doesNotSupportUnit('endOf', 'LINE');
      case Unit.PAGE:
        doesNotSupportUnit('endOf', 'PAGE');
      case Unit.PARAGRAPH: {
        var position = this.end;
        while (position < document.length) {
          if (document.charCodeAt_(position) == '\n')
            break;
          ++position;
        }
        this.end = position;
        break;
      }
      case Unit.SCREEN:
        doesNotSupportUnit('endOf', 'SCREEN');
      case Unit.SENTENCE:
        NYI('startof', 'SENTENCE');
      case Unit.WINDOW:
        doesNotSupportUnit('startof', 'WINDOW');
      case Unit.WORD: {
        if (this.end == document.length)
          break;
        var position = this.end;
        var word_class = wordClassOf(document.charCodeAt_(position));
        if (word_class == WordClass.BLANK)
          break;
        for (;;) {
          ++position;
          if (position == document.length)
            break;
          var word_class2 = wordClassOf(document.charCodeAt_(position));
          if (word_class != word_class2) {
            break;
          }
        }
        this.end = position;
        break;
      }
      default:
        throw TypeError('Invalid unit: ' + unit);
    }
    switch (alter) {
      case Alter.EXTEND:
        break;
      case Alter.MOVE:
        this.start = this.end;
        break;
      default:
        throw TypeError('Invalid alter: ' + alter);
    }
  };

  /**
   * Move start position of Range at start of specified unit.
   * @this {Range}
   * @param {Unit} unit.
   * @param {Alter} alter, optional default is Alter.MOVE.
   */
  Range.prototype.startOf = function(unit, alter) {
    alter = arguments.length == 1 ? Alter.MOVE : alter;
    var document = this.document;
    switch (unit) {
      case Unit.CHARACTER:
        doesNotSupportUnit('startOf', 'CHARACTER');
      case Unit.DOCUMENT:
        this.start = 0;
        break;
      case Unit.LINE:
        doesNotSupportUnit('startOf', 'LINE');
      case Unit.PAGE:
        doesNotSupportUnit('startOf', 'PAGE');
      case Unit.PARAGRAPH: {
        var position = this.start;
        while (position > 0) {
          --position;
          if (document.charCodeAt_(position) == '\n') {
            ++position;
            break;
          }
        }
        this.start = position;
        break;
      }
      case Unit.SCREEN:
        doesNotSupportUnit('startOf', 'SCREEN');
      case Unit.SENTENCE:
        NYI('startof', 'SENTENCE');
      case Unit.WINDOW:
        doesNotSupportUnit('startof', 'WINDOW');
      case Unit.WORD: {
        var position = this.start;
        if (!position)
          break;
        var word_class = wordClassOf(document.charCodeAt_(position));
        if (word_class == WordClass.BLANK)
          break;
        while (position) {
          --position;
          var word_class2 = wordClassOf(document.charCodeAt_(position));
          if (word_class != word_class2) {
            ++position;
            break;
          }
        }
        this.start = position;
        break;
      }
      default:
        throw TypeError('Invalid unit: ' + unit);
    }
    switch (alter) {
      case Alter.EXTEND:
        break;
      case Alter.MOVE:
        this.end = this.start;
        break;
      default:
        throw TypeError('Invalid alter: ' + alter);
    }
  };

  /**
   * @this {Range}
   */
  Range.prototype.toLocaleLowerCase = function() {
    this.text = this.text.toLocaleLowerCase();
  };
  
  /**
   * @this {Range}
   */
  Range.prototype.toLocaleUpperCase = function() {
    this.text = this.text.toLocaleUpperCase();
  };
  
  /**
   * @this {Range}
   */
  Range.prototype.toLowerCase = function() {
    this.text = this.text.toLowerCase();
  };
  
  /**
   * @this {Range}
   */
  Range.prototype.toUpperCase = function() {
    this.text = this.text.toUpperCase();
  };
})();
