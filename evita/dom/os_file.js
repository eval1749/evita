// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {

  /**
   * @param {string} file_name
   * @return {!Promise.<!Os.File.Info>}
   */
  Os.File.stat = function(file_name) {
    var deferred = Promise.defer();
    Os.File.stat_(file_name, function(info) {
      if (info.errorCode)
        deferred.reject(info.errorCode);
      else
        deferred.resolve(info);
    });
    return deferred.promise;
  };
})();
