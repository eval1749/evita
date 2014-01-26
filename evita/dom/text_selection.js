// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @param {!TextSelection} selection
   * @param {!Alter} alter
   * @param {number} anchor
   * @param {number} focus
   * @return {!TextSelection}
   */
  function updateSelection(selection, alter, anchor, focus) {
    switch (alter) {
      case Alter.EXTEND: {
        selection.range.collapseTo(anchor);
        if (anchor < focus) {
          selection.range.end = focus;
          selection.startIsActive = false;
        } else {
          selection.range.start = focus;
          selection.startIsActive = true;
        }
        return selection;
      }
      case Alter.MOVE:
        selection.range.collapseTo(focus);
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
    var alter = arguments.length >= 2 ? /** @type{Alter} */(opt_alter) :
                                        Alter.MOVE;
    if (this.startIsActive)
      this.range.collapseTo(this.range.end);
    var anchor = this.range.start;
    var focus = this.range.end;
    this.endOf(unit);
    var new_focus = this.range.end;
    if (unit == Unit.LINE || unit == Unit.WINDOW_LINE) {
      this.range.moveEndWhile(' \t', Count.BACKWARD);
      // Skip trailing whitespace if
      //   - The selection were at end of line.
      //   - The selection was at middle of line.
      if (focus == new_focus || focus != this.range.end)
        new_focus = this.range.end;
    }
    return updateSelection(this, alter, anchor, new_focus);
  };

  /**
   * Move end position of TextSelection at end of specified unit.
   * @this {!TextSelection}
   * @param {Unit} unit.
   * @param {Alter=} opt_alter, default is Alter.MOVE.
   * @return {!TextSelection}
   */
  TextSelection.prototype.endOf = function(unit, opt_alter) {
    var alter = arguments.length >= 2 ? /** @type{Alter} */(opt_alter) :
                                        Alter.MOVE;
    var anchor = this.range.start;
    switch (unit) {
      case Unit.WINDOW:
        return updateSelection(this, alter, anchor,
            Math.min(this.window.compute_(
                        TextWindowComputeMethod.END_OF_WINDOW),
                     this.document.length));
      case Unit.WINDOW_LINE:
        return updateSelection(this, alter, anchor,
            this.window.compute_(TextWindowComputeMethod.END_OF_WINDOW_LINE,
                                 this.range.end));
       return this;
    }
    Range.prototype.endOf.call(this.range, unit);
    return updateSelection(this, alter, anchor, this.range.end);
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
    var focus = this.range.start;
    this.startOf(unit);
    var new_focus = this.range.start;
    if (unit == Unit.LINE || unit == Unit.WINDOW_LINE) {
      this.range.moveStartWhile(' \t', Count.FORWARD);
      // Skip leading whitespace if
      //   - The selection were at end of line.
      //   - The selection was at middle of line.
      if (focus == new_focus || focus != this.range.start)
        new_focus = this.range.start;
    }
    return updateSelection(this, alter, anchor, new_focus);
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
    var anchor = this.range.end;
    switch (unit) {
      case Unit.WINDOW:
        return updateSelection(this, alter, anchor,
                 this.window.compute_(
                    TextWindowComputeMethod.START_OF_WINDOW));
      case Unit.WINDOW_LINE:
        return updateSelection(this, alter, anchor,
                 this.window.compute_(
                    TextWindowComputeMethod.START_OF_WINDOW_LINE,
                    this.range.start));
    }
    this.range.startOf(unit);
    return updateSelection(this, alter, anchor, this.range.start);
  };
})();
