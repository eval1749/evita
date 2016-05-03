// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var goog = {};

/** @param {string} name */
goog.require = function(name) {
  if (name in global)
    return;
  Editor.loadModule(name);
};

/** @param {string} qualifiedName */
goog.provide = function(qualifiedName) {
  /** @type {!Object} */
  let runner = global;
  qualifiedName.split('.').forEach(name => {
    if (!(name in runner))
      runner[name] = {};
    runner = /** @type {!Object} */ (runner[name]);
  });
};


/** @param {!function(): ?} fn */
goog.scope = function(fn) {
  fn();
};

goog.provide("core");
goog.provide("text");
