// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('base.Logger');

/** @constructor */
base.Logger = function() {};

/** @type {number} */
base.Logger.prototype.verbose;

/**
 * @param {number} level
 * @param {...*} args
 */
base.Logger.prototype.log = function(level, args) {};
