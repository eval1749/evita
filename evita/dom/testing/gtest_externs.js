// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @type {!Object} */
var gtest;

/**
 * @param {*} value1
 * @param {*} value2
 * @param {string} description
 */
gtest.expectEqual = function(value1, value2, description) {};

/**
 * @param {*} condition
 * @param {string} description
 */
gtest.expectFalse = function(condition, description) {};

/**
 * @param {*} condition
 * @param {string} description
 */
gtest.expectTrue = function(condition, description) {};

/**
 * @param {string} description
 */
gtest.fail = function(description) {};

/** @return {void} */
gtest.succeed = function() {};
