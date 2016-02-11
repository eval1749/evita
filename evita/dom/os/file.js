// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @constructor
   */
  Os.File.Error = (function() {
    return function Error(winLastError) { this.winLastError = winLastError; }
  })();

  /**
   * @constructor
   */
  Os.File.Info = (function() { return function Info() {}; })();
})();
