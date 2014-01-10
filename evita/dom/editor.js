// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  /**
   * @param {Window} window.
   * @param {string} dirname.
   * @return {Promise}
   */
  Editor.getFilenameForLoad = function(window, dirname) {
    var resolved;
    var promise = new Promise(function(r) { resolved = r; });
    Editor.getFilenameForLoad_(window, dirname, resolved);
    return promise;
  };

  /**
   * @param {Window} window.
   * @param {string} dirname.
   * @return {Promise}
   */
  Editor.getFilenameForSave = function(window, dirname) {
    var resolved;
    var promise = new Promise(function(r) { resolved = r; });
    Editor.getFilenameForSave_(window, dirname, resolved);
    return promise;
  };
})();
