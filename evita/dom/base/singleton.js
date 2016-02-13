// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /** @const @type {!Map<!Function, !Object>} */
  const singletonMap = new Map();

  //////////////////////////////////////////////////////////////////////
  //
  // Singleton
  //
  class Singleton {
    /**
     * @param {!Function} constructor
     * @return {!Object}
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

  base.Singleton = Singleton;
})();

