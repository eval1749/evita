// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('base.Singleton');

goog.scope(function() {

/** @const @type {!Map<!Function, !Object>} */
const singletonMap = new Map();

//////////////////////////////////////////////////////////////////////
//
// Singleton
//
class Singleton {
  /**
   * @template T
   * @param {function(new: T)} constructor
   * @return {!T}
   */
  static get(constructor) {
    const instance = singletonMap.get(constructor);
    if (instance)
      return instance;
    const newInstance = new constructor();
    singletonMap.set(constructor, newInstance);
    return newInstance;
  }
}

/** @constructor */
base.Singleton = Singleton;
});
