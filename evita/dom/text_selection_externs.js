// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview TextSelection
 * @externs
 */

/** @expose @type {number} */
TextSelection.prototype.active;

/** @expose @type {!Document} */
TextSelection.prototype.document;

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
