// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @param {!TextSelection} selection
   * @param {!Alter} alter
   * @param {number} anchor
   * @param {number} active_position
   * @return {!TextSelection}
   */
  function updateSelection(selection, alter, anchor, active_position) {
    switch (alter) {
      case Alter.EXTEND: {
        selection.range.collapseTo(anchor);
        if (anchor < active_position) {
          selection.range.end = active_position;
          selection.startIsActive = false;
        } else {
          selection.range.start = active_position;
          selection.startIsActive = true;
        }
        return selection;
      }
      case Alter.MOVE:
        selection.range.collapseTo(active_position);
        return selection;
    }
    throw 'Invalid ALTER: ' + alter;
  }

  /**
   * @param {Unit} unit
   * @param {Alter} opt_alter, default is Alter.MOVE
   * @return {!TextSelection}
   */
  TextSelection.prototype.endKey = function(unit, opt_alter) {
    var alter = arguments.length >= 2 ? opt_alter : Alter.MOVE;
    if (this.startIsActive)
      this.range.collapseTo(this.range.end);
    var start = this.range.end;
    this.endOf(unit);
    var new_end = this.range.end;
    if (unit == Unit.LINE || unit == Unit.WINDOW_LINE) {
      this.range.moveEndWhile(' \t', Count.BACKWARD);
      // Skip trailing whitespace if
      //   - The selection were at end of line.
      //   - The selection was at middle of line.
      if (start == new_end || start != this.range.end)
        new_end = this.range.end;
    }
    if (alter == Alter.EXTEND) {
      if (new_end < start) {
        this.range.start = new_end;
        this.range.end = start;
        this.startIsActive = true;
      } else {
        this.range.start = start;
        this.range.end = new_end;
        this.startIsActive = false;
      }
    } else {
      this.range.collapseTo(new_end);
    }
    return this;
  };

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

  /**
   * @param {Unit} unit, one of DOCUMENT, LINE, or WINDOW_LINE.
   * @param {!Alter=} opt_alter, default is Alter.MOVE
   * @return {!TextSelection}
   */
  TextSelection.prototype.homeKey = function(unit, opt_alter) {
    var alter = arguments.length >= 2 ? /** @type{Alter} */(opt_alter) :
                                        Alter.MOVE;
    if (!this.startIsActive)
      this.range.collapseTo(this.range.start);
    var anchor = this.range.end;
    var start = this.range.start;
    this.startOf(unit);
    var new_start = this.range.start;
    if (unit == Unit.LINE || unit == Unit.WINDOW_LINE) {
      this.range.moveStartWhile(' \t', Count.FORWARD);
      // Skip leading whitespace if
      //   - The selection were at end of line.
      //   - The selection was at middle of line.
      if (start == new_start || start != this.range.start)
        new_start = this.range.start;
    }
    return updateSelection(this, alter, anchor, new_start);
  };

  /**
   * Move start position of TextSelection at end of specified unit.
   * @this {!TextSelection}
   * @param {Unit} unit, except for CHARACTER and SCREEN,
   * @param {!Alter=} opt_alter, default is Alter.MOVE.
   * @return {!TextSelection}
   */
  TextSelection.prototype.startOf = function(unit, opt_alter) {
    var alter = arguments.length >= 2 ? /** @type{Alter} */(opt_alter) :
                                        Alter.MOVE;
    switch (unit) {
      case Unit.WINDOW:
        return updateSelection(this, alter, this.range.end,
                 this.window.compute_(
                    TextWindowComputeMethod.START_OF_WINDOW));
      case Unit.WINDOW_LINE:
        return updateSelection(this, alter, this.range.end,
                 this.window.compute_(
                    TextWindowComputeMethod.START_OF_WINDOW_LINE,
                    this.range.end));
    }
    Range.prototype.startOf.call(this.range, unit, alter);
    return this;
  };
})();
