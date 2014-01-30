// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @param {!TextSelection} selection
   * @param {Unit} unit
   * @param {number} count
   * @param {number} position
   */
  function computeMotion(selection, unit, count, position) {
    if (unit == Unit.SCREEN) {
      if (!selection.goal_point_)
        throw 'Goal X point must be initialized.';
      return selection.window.compute_(TextWindowComputeMethod.MOVE_SCREEN,
          position, count, selection.goal_point_);
    }

    if (unit == Unit.WINDOW) {
      return selection.window.compute_(TextWindowComputeMethod.MOVE_WINDOW,
                                       position, count);
    }

    if (unit == Unit.WINDOW_LINE) {
      if (!selection.goal_point_)
        throw 'Goal X point must be initialized.';
      return selection.window.compute_(
            TextWindowComputeMethod.MOVE_WINDOW_LINE, position, count,
            selection.goal_point_);
    }

    return selection.range.document.computeMotion_(unit, count, position);
  }

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
        // TODO(yosi) This is temporary hack for forgetting goal x-point.
        // Once, we implment new goal x-point tracking, we should remove
        // this hack.
        selection.startIsActive = true;
        return selection;
    }
    throw 'Invalid ALTER: ' + alter;
  }

  /**
   * @param {!TextSelection} selection
   * @param {!Unit} unit
   */
  function updateGoalX(selection, unit) {
    if (unit != Unit.SCREEN && unit != Unit.WINDOW_LINE) {
      selection.goal_point_ = undefined;
      return;
    }
    var focus = selection.startIsActive ? selection.range.start :
                                          selection.range.end;
    if (selection.goal_position_ == focus && selection.goal_point_)
      return;
    selection.goal_point_ = selection.window.mapPositionToPoint_(focus);
    selection.goal_position_ = focus;
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
   * @this {!TextSelection}
   * @param {Unit} unit
   * @param {number=} opt_count, default is one.
   * @param {Alter=} opt_alter, defualt is Alter.MOVE
   * @return {!TextSelection}
   */
  TextSelection.prototype.modify= function(unit, opt_count, opt_alter) {
    var count = arguments.length >= 2 ? /** @type {number} */(opt_count) : 1;
    var alter = arguments.length >= 3 ? /** @type {Alter} */(opt_alter) :
                                        Alter.MOVE;
    updateGoalX(this, unit);
    switch (alter) {
      case Alter.EXTEND: {
        if (this.range.start == this.range.end)
          this.startIsActive = count < 0;
        if (this.startIsActive)
          this.moveStart(unit, count);
        else
          this.moveEnd(unit, count);
        break;
      }
      case Alter.MOVE:
        if (this.range.start == this.range.end)
            this.move(unit, count);
        else if (count > 0)
          this.range.collapseTo(this.range.start);
        else
          this.range.collapseTo(this.range.end);
        break;
      default:
        throw 'Invalid alter: ' + alter;
    }
    if (this.goal_point_) {
      this.goal_position_ = this.startIsActive ? this.range.start :
                                                 this.range.end;
    }
    return this;
  };

  /**
   * @this {!TextSelection}
   * @param {Unit} unit
   * @param {number=} opt_count, defualt is one.
   * @return {!TextSelection}
   */
  TextSelection.prototype.move = function(unit, opt_count) {
    var count = arguments.length >= 2 ? /** @type {number} */(opt_count) : 1;
    if (this.startIsActive) {
      return updateSelection(this, Alter.MOVE, this.range.end,
          computeMotion(this, unit, count, this.range.start));
    }

    return updateSelection(this, Alter.MOVE, this.range.start,
          computeMotion(this, unit, count, this.range.end));
  };

  /**
   * @this {!TextSelection}
   * @param {Unit} unit
   * @param {number=} opt_count, defualt is one.
   * @return {!TextSelection}
   */
  TextSelection.prototype.moveEnd = function(unit, opt_count) {
    var count = arguments.length >= 2 ? /** @type {number} */(opt_count) : 1;
    var position = computeMotion(this, unit, count, this.range.end);
    if (position < this.range.start)
      this.range.collapseTo(position);
    else
      this.range.end = position;
    return this;
  };

  /**
   * @this {!TextSelection}
   * @param {Unit} unit
   * @param {number=} opt_count, defualt is one.
   * @return {!TextSelection}
   */
  TextSelection.prototype.moveStart = function(unit, opt_count) {
    var count = arguments.length >= 2 ? /** @type {number} */(opt_count) : 1;
    var position = computeMotion(this, unit, count, this.range.start);
    if (position > this.range.end)
      this.range.collapseTo(position);
    else
      this.range.start = position;
    return this;
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
