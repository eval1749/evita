// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {
/**
 * @param {!TextSelection} selection
 * @param {Unit} unit
 * @param {number} count
 * @param {number} offset
 * @return {number}
 */
function computeMotion(selection, unit, count, offset) {
  if (unit === Unit.SCREEN) {
    if (!selection.goal_point_)
      throw 'Goal X point must be initialized.';
    return selection.window.compute_(
        TextWindowComputeMethod.MOVE_SCREEN, offset, count,
        selection.goal_point_);
  }

  if (unit === Unit.WINDOW) {
    return selection.window.compute_(
        TextWindowComputeMethod.MOVE_WINDOW, offset, count);
  }

  if (unit === Unit.WINDOW_LINE) {
    if (!selection.goal_point_)
      throw 'Goal X point must be initialized.';
    return selection.window.compute_(
        TextWindowComputeMethod.MOVE_WINDOW_LINE, offset, count,
        selection.goal_point_);
  }

  return selection.range.document.computeMotion_(unit, count, offset);
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
      // TODO(eval1749): This is temporary hack for forgetting goal x-point.
      // Once, we implement new goal x-point tracking, we should remove
      // this hack.
      selection.startIsActive = true;
      return selection;
  }
  throw `Invalid ALTER: ${alter}`;
}

/**
 * @param {!TextSelection} selection
 * @param {!Unit} unit
 */
function updateGoalX(selection, unit) {
  if (unit !== Unit.SCREEN && unit !== Unit.WINDOW_LINE) {
    selection.goal_point_ = null;
    return;
  }
  /** @const @type {number} */
  const focus =
      selection.startIsActive ? selection.range.start : selection.range.end;
  if (selection.goal_position_ === focus && selection.goal_point_)
    return;
  /** @const @type {!Rect} */
  const rect = selection.window.hitTestTextPosition_(focus);
  selection.goal_point_ = rect.leftTop;
  selection.goal_position_ = focus;
}

/**
 * @this {!TextSelection}
 * @param {Unit} unit
 * @param {Alter=} alter, default is Alter.MOVE
 * @return {!TextSelection}
 */
function endKey(unit, alter = Alter.MOVE) {
  if (this.startIsActive)
    this.range.collapseTo(this.range.end);
  /** @const @type {number} */
  const anchor = this.range.start;
  /** @const @type {number} */
  const focus = this.range.end;
  this.endOf(unit);
  /** @const @type {number} */
  const newFocus = this.range.end;
  if (unit === Unit.LINE || unit === Unit.WINDOW_LINE) {
    // Skip trailing whitespace if
    //   - The selection were at end of line.
    //   - The selection was at middle of line.
    this.range.moveEndWhile(' \t', Count.BACKWARD);
    if (focus === newFocus || focus !== this.range.end)
      return updateSelection(this, alter, anchor, this.range.end);
  }
  return updateSelection(this, alter, anchor, newFocus);
}

/**
 * Move end position of TextSelection at end of specified unit.
 * @this {!TextSelection}
 * @param {Unit} unit
 * @param {Alter=} alter, default is Alter.MOVE.
 * @return {!TextSelection}
 */
function endOf(unit, alter = Alter.MOVE) {
  /** @const @type {number} */
  const anchor = this.range.start;
  switch (unit) {
    case Unit.WINDOW:
      return updateSelection(
          this, alter, anchor,
          Math.min(
              this.window.compute_(TextWindowComputeMethod.END_OF_WINDOW),
              this.document.length));
    case Unit.WINDOW_LINE:
      return updateSelection(
          this, alter, anchor,
          this.window.compute_(
              TextWindowComputeMethod.END_OF_WINDOW_LINE, this.range.end));
  }
  this.range.endOf(unit);
  return updateSelection(this, alter, anchor, this.range.end);
}

/**
 * @this {!TextSelection}
 * @param {Unit} unit, one of DOCUMENT, LINE, or WINDOW_LINE.
 * @param {!Alter=} alter, default is Alter.MOVE
 * @return {!TextSelection}
 */
function homeKey(unit, alter = Alter.MOVE) {
  if (!this.startIsActive)
    this.range.collapseTo(this.range.start);
  /** @const @type {number} */
  const anchor = this.range.end;
  /** @const @type {number} */
  const focus = this.range.start;
  this.startOf(unit);
  /** @const @type {number} */
  const newFocus = this.range.end;
  if (unit === Unit.LINE || unit === Unit.WINDOW_LINE) {
    // Skip leading whitespace if
    //   - The selection were at end of line.
    //   - The selection was at middle of line.
    this.range.moveStartWhile(' \t', Count.FORWARD);
    if (focus === newFocus || focus !== this.range.start)
      return updateSelection(this, alter, anchor, this.range.start);
  }
  return updateSelection(this, alter, anchor, newFocus);
}

/**
 * @this {!TextSelection}
 * @param {Unit} unit
 * @param {number=} count, default is one.
 * @param {Alter=} alter, default is Alter.MOVE
 * @return {!TextSelection}
 */
function modify(unit, count = 1, alter = Alter.MOVE) {
  updateGoalX(this, unit);
  switch (alter) {
    case Alter.EXTEND: {
      if (this.range.start === this.range.end)
        this.startIsActive = count < 0;
      if (this.startIsActive)
        this.moveStart(unit, count);
      else
        this.moveEnd(unit, count);
      break;
    }
    case Alter.MOVE:
      if (this.range.start === this.range.end)
        this.move(unit, count);
      else if (count < 0)
        this.range.collapseTo(this.range.start);
      else if (count > 0)
        this.range.collapseTo(this.range.end);
      break;
    default:
      throw 'Invalid alter: ' + alter;
  }
  if (this.goal_point_) {
    this.goal_position_ =
        this.startIsActive ? this.range.start : this.range.end;
  }
  return this;
}

/**
 * @this {!TextSelection}
 * @param {Unit} unit
 * @param {number=} count, default is one.
 * @return {!TextSelection}
 */
function move(unit, count = 1) {
  if (this.startIsActive) {
    return updateSelection(
        this, Alter.MOVE, this.range.end,
        computeMotion(this, unit, count, this.range.start));
  }

  return updateSelection(
      this, Alter.MOVE, this.range.start,
      computeMotion(this, unit, count, this.range.end));
}

/**
 * @this {!TextSelection}
 * @param {Unit} unit
 * @param {number=} count, default is one.
 * @return {!TextSelection}
 */
function moveEnd(unit, count = 1) {
  /** @const @type {number} */
  const offset = computeMotion(this, unit, count, this.range.end);
  if (offset < this.range.start) {
    this.range.collapseTo(offset);
    return this;
  }
  this.range.end = offset;
  return this;
}

/**
 * @this {!TextSelection}
 * @param {Unit} unit
 * @param {number=} count, default is one.
 * @return {!TextSelection}
 */
function moveStart(unit, count = 1) {
  /** @const @type {number} */
  const offset = computeMotion(this, unit, count, this.range.start);
  if (offset > this.range.end) {
    this.range.collapseTo(offset);
    return this;
  }
  this.range.start = offset;
  return this;
}

/**
 * Move start position of TextSelection at end of specified unit.
 * @this {!TextSelection}
 * @param {Unit} unit, except for CHARACTER and SCREEN,
 * @param {!Alter=} alter, default is Alter.MOVE.
 * @return {!TextSelection}
 */
function startOf(unit, alter = Alter.MOVE) {
  /** @const @type {number} */
  const anchor = this.range.end;
  switch (unit) {
    case Unit.WINDOW:
      return updateSelection(
          this, alter, anchor,
          this.window.compute_(TextWindowComputeMethod.START_OF_WINDOW));
    case Unit.WINDOW_LINE:
      return updateSelection(
          this, alter, anchor,
          this.window.compute_(
              TextWindowComputeMethod.START_OF_WINDOW_LINE, this.range.start));
  }
  this.range.startOf(unit);
  return updateSelection(this, alter, anchor, this.range.start);
}

Object.defineProperties(TextSelection.prototype, {
  endKey: {value: endKey},
  endOf: {value: endOf},
  homeKey: {value: homeKey},
  modify: {value: modify},
  move: {value: move},
  moveEnd: {value: moveEnd},
  moveStart: {value: moveStart},
  startOf: {value: startOf},
});

});
