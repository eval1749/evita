// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

(function() {
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
    this.end = this.document.computeEndOf_(unit, this.end);
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
    this.start = this.document.computeStartOf_(unit, this.start);
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
