// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @this {!Timer}
   * @param {number} delayMs
   * @param {function()} callback
   * @param {?=} opt_reciver
   */
  function timerStart(delayMs, callback, opt_reciver) {
    let reciver = opt_reciver === undefined ? this : opt_reciver;
    this.startInternal(delayMs, callback.bind(reciver));
  }
  Object.defineProperty(Timer.prototype, 'start', {value: timerStart});
})();
