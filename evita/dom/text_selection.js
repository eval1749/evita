// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * Move end position of TextSelection at end of specified unit.
   * @this {!TextSelection}
   * @param {Unit} unit.
   * @param {Alter=} opt_alter, default is Alter.MOVE.
   * @return {!TextSelection}
   */
  TextSelection.prototype.endOf = function(unit, opt_alter) {
    var alter = arguments.length >= 2 ? opt_alter : Alter.MOVE;
    switch (unit) {
      case Unit.WINDOW:
        this.range.end = Math.min(this.window.compute_(
                                    TextWindowComputeMethod.END_OF_WINDOW),
                                  this.document.length);
        break;
      case Unit.WINDOW_LINE:
        this.range.end = this.window.compute_(
            TextWindowComputeMethod.END_OF_WINDOW_LINE, this.range.end);
        break;
      default:
       Range.prototype.endOf.call(this.range, unit, alter);
       return this;
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
    return this;
  };
})();
