// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @constructor
   */
  Os.File.Error = (function() {
    return function Error(winLastError) {
      this.winLastError = winLastError;
    }
  })();

  /**
   * @param {string} file_name
   * @return {!Promise.<!Os.File.Info>}
   */
  Os.File.stat = function(file_name) {
    var deferred = Promise.defer();
    Os.File.stat_(file_name, function(info) {
      if (info.errorCode)
        deferred.reject(new Os.File.Error(info.errorCode));
      else
        deferred.resolve(info);
    });
    return deferred.promise;
  };

  /**
   * @expose
   * @param {!ArrayBufferView} array_buffer_view
   * @return {!Promise.<number|!Os.File.Error>}
   */
  Os.File.prototype.read = function(array_buffer_view) {
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
  Os.File.prototype.write = function(array_buffer_view) {
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
