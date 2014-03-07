// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @expose
   * @param {!ArrayBufferView} array_buffer_view
   * @return {!Promise.<number|!Os.File.Error>}
   */
  Os.FileHandle.prototype.read = function(array_buffer_view) {
    var deferred = Promise.defer();
    this.read_(array_buffer_view, function(num_transfered, error_code) {
      if (error_code)
        deferred.reject(new Os.File.Error(error_code))
      else
        deferred.resolve(num_transfered)
    });
    return deferred.promise;
  };

  /**
   * @expose
   * @param {!ArrayBufferView} array_buffer_view
   * @return {!Promise.<number|!Os.File.Error>}
   */
  Os.FileHandle.prototype.write = function(array_buffer_view) {
    var deferred = Promise.defer();
    this.write_(array_buffer_view, function(num_transfered, error_code) {
      if (error_code)
        deferred.reject(new Os.File.Error(error_code))
      else
        deferred.resolve(num_transfered)
    });
    return deferred.promise;
  };
})();
