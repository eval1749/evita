// Copyright (c) 2015 Project Vogue. All rights reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file


goog.scope(function() {
/** @const @type {!Array.<!function():*>} */
const initializers = [];

class Initializer {
  static initialize() { initializers.forEach(initializer => initializer()); }
  /**
   * @param {!function()} callable
   */
  static register(callable) { initializers.push(callable); }
}

/** @constructor */
core.Initializer = Initializer;
});
