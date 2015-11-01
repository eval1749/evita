// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  //////////////////////////////////////////////////////////////////////
  //
  // Runnable
  //
  class Runnable {
    /** @return {undefined} */
    run() {
      const className = this.constructor.name;
      throw new Error(`${className} must implement run() method.`);
    }
  }

  global.Runnable = Runnable;
})();
