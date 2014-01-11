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
   * Move start position of Range at start of specified unit.
   * @this {Range}
   * @param {Unit} unit.
   */
  Range.prototype.startOf = function(unit) {
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
