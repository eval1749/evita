// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview TextSelection
 * @externs
 */

/** @expose @type {!TextOffset} */
TextSelection.prototype.anchorOffset;

/** @expose @type {!Document} */
TextSelection.prototype.document;

/** @expose @type {!TextOffset} */
TextSelection.prototype.focusOffset;

/**
 * @param {Unit} unit
 * @param {Alter} opt_alter, default is Alter.MOVE
 * @return {!TextSelection}
 */
TextSelection.prototype.endKey = function(unit, opt_alter) {};

/**
 * @param {Unit} unit
 * @param {Alter=} opt_alter, default is Alter.MOVE.
 */
TextSelection.prototype.endOf = function(unit, opt_alter) {};

// Note: We use |undefined| as marker of no goal x point to avoid
// initialization of |goal_point_| in C++.
/** @type {!Point|undefined} */
TextSelection.prototype.goal_point_;

/** @type {!TextOffset|undefined} */
TextSelection.prototype.goal_position_;

/**
 * @param {Unit} unit, one of DOCUMENT, LINE, or WINDOW_LINE.
 * @param {Alter} opt_alter, default is Alter.MOVE
 * @return {!TextSelection}
 */
TextSelection.prototype.homeKey = function(unit, opt_alter) {};

/**
 * @this {!TextSelection}
 * @param {Unit} unit
 * @param {number=} opt_count, default is one.
 * @param {Alter=} opt_alter, defualt is Alter.MOVE
 * @return {!TextSelection}
 */
TextSelection.prototype.modify = function(unit, opt_count, opt_alter) {};

/**
 * @param {Unit} unit
 * @param {number=} opt_count, defualt is one.
 * @return {!TextSelection}
 */
TextSelection.prototype.move = function(unit, opt_count) {};

/**
 * @param {Unit} unit
 * @param {number=} opt_count, defualt is one.
 * @return {!TextSelection}
 */
TextSelection.prototype.moveEnd = function(unit, opt_count) {};

/**
 * @param {Unit} unit
 * @param {number=} opt_count, defualt is one.
 * @return {!TextSelection}
 */
TextSelection.prototype.moveStart = function(unit, opt_count) {};

/** @expose @type {!Range} */
TextSelection.prototype.range;

/** @expose @type {boolean} */
TextSelection.prototype.startIsActive;

/**
 * @param {Unit} unit
 * @param {Alter=} opt_alter, default is Alter.MOVE.
 */
TextSelection.prototype.startOf = function(unit, opt_alter) {};

/** @expose @type {!TextWindow} */
TextSelection.prototype.window;
