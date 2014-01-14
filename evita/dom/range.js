// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

(function() {
  /**
   * Capitalize range.
   * @this {Range}
   * @return {Range}
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
        break;
      }
    }
    return this;
  };

  /**
   * Move end position of Range at end of specified unit.
   * @this {Range}
   * @param {Unit} unit.
   * @param {Alter=} opt_alter, default is Alter.MOVE.
   * @return {Range}
   */
  Range.prototype.endOf = function(unit, opt_alter) {
    var alter = arguments.length >= 2 ? opt_alter : Alter.MOVE;
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
    return this;
  };

  /**
   * @this {Range}
   * @param {Unit} unit.
   * @param {number=} opt_count, defualt is one.
   * @return {Range}
   */
  Range.prototype.move = function(unit, opt_count) {
    var count = arguments.length >= 2 ? opt_count : 1;
    var position = count > 0 ? this.end : this.start;
    this.collapseTo(this.document.computeMotion_(unit, count, position));
    return this;
  };

  /**
   * @this {Range}
   * @param {Unit} unit.
   * @param {number=} opt_count, defualt is one.
   * @return {Range}
   */
  Range.prototype.moveEnd = function(unit, opt_count) {
    var count = arguments.length >= 2 ? opt_count : 1;
    var position = this.document.computeMotion_(unit, count, this.end);
    if (position >= this.start)
      this.end = position;
    else
      this.collapseTo(position);
    return this;
  };

  /**
   * @this {Range}
   * @param {Unit} unit.
   * @param {number=} opt_count, defualt is one.
   * @return {Range}
   */
  Range.prototype.moveStart = function(unit, opt_count) {
    var count = arguments.length >= 2 ? opt_count : 1;
    var position = this.document.computeMotion_(unit, count, this.start);
    if (position <= this.end)
      this.start = position;
    else
      this.collapseTo(position);
    return this;
  };

  /**
   * Move start position of Range at start of specified unit.
   * @this {Range}
   * @param {Unit} unit.
   * @param {Alter=} opt_alter, default is Alter.MOVE.
   */
  Range.prototype.startOf = function(unit, opt_alter) {
    var alter = arguments.length >= 2 ? opt_alter : Alter.MOVE;
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
   * @return {Range}
   */
  Range.prototype.toLocaleLowerCase = function() {
    this.text = this.text.toLocaleLowerCase();
    return this;
  };
  
  /**
   * @this {Range}
   * @return {Range}
   */
  Range.prototype.toLocaleUpperCase = function() {
    this.text = this.text.toLocaleUpperCase();
    return this;
  };
  
  /**
   * @this {Range}
   * @return {Range}
   */
  Range.prototype.toLowerCase = function() {
    this.text = this.text.toLowerCase();
    return this;
  };
  
  /**
   * @this {Range}
   * @return {Range}
   */
  Range.prototype.toUpperCase = function() {
    this.text = this.text.toUpperCase();
    return this;
  };
})();
