// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @param {function(...): !Generator.<?>} makeGenerator
 * @return {function(...): Promise.<?>}
 */
function async(makeGenerator) {
  return function () {
    const generator = makeGenerator.apply(this, arguments);

    /**
     * @param {{done: boolean, value: ?}} result
     * @return {Promise.<?>}
     */
    function handle(result){
      if (result.done)
        return Promise.resolve(result.value);

      return Promise.resolve(result.value).then(function(res) {
        return handle(generator.next(res));
      }, function(err){
        return handle(generator.throw(err));
      });
    }

    try {
      return handle(generator.next());
    } catch (ex) {
      return Promise.reject(ex);
    }
  }
}
