// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

// Ecmascript6: Array.prototype.find
if (!Array.prototype.find) {
  /**
   * @param {function(*) : boolean} callback
   * @param {*=} opt_thisObject
   * @return {Object|undefined}
   */
  Array.prototype.find = function(callback, opt_thisObject) {
    var thisObject = opt_thisObject || this;
    for (var i = 0; i < this.length; ++i) {
      var element = this[i];
      if (callback.call(thisObject, element))
        return element;
    }
    return undefined;
  };
}
