// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('base');

/** @constructor */
base.Singleton = function() {};

/**
 * @template T
 * @param {function(new: T)} constructor
 * @return {!T}
 */
base.Singleton.get = function(constructor) {};
