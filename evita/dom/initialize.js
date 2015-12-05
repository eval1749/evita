// Copyright (c) 2015 Project Vogue. All rights reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

(function() {
  /** @const @type {!Array.<!function():*>} */
  const initializers = []

  function $initialize(initializer) {
    initializers.push(initializer);
  }

  function initialize() {
    initializers.forEach(initializer => initializer());
  }

  Object.defineProperties(global, {
    $initialize: {value: $initialize},
    initialize: {value: initialize}
  });
})();
