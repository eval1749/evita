// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {

  /**
   * @param {string=} opt_filename
   * @return {!Promise.<number>}
   */
  Document.prototype.save = function(opt_filename) {
    var document = this;
    if (!arguments.length) {
      if (document.filename == '')
        throw 'Document isn\'t bound to file.';
    } else {
      var filename = /** @type{string} */(opt_filename);
      // TODO(yosi) FilePath.fullPath() will return Promise.
      var absolute_filename = FilePath.fullPath(filename);
      var present = Document.findFile(absolute_filename);
      if (present && present !== this)
        throw filename + ' is already bound to ' + present;
      document.filename = absolute_filename;
    }

    var deferred = Promise.defer();
    var filename = document.filename;
    document.obsolete = Document.Obsolete.CHECKING;
    Editor.messageBox(null, 'Saving to ' + document.filename,
                      MessageBox.ICONINFORMATION);
    document.save_(filename, function(error_code) {
      if (!error_code)
        deferred.resolve(error_code);
      else
        deferred.reject(error_code);
    });

    return deferred.promise.then(function(error_code) {
      Editor.messageBox(null, 'Saved to ' + filename,
                        MessageBox.ICONINFORMATION);
      document.lastStatusCheckTime_ = new Date();
      document.obsolete = Document.Obsolete.NO;
      document.dispatchEvent(new DocumentEvent('save'));
      return deferred.promise;
    }).catch(function(reason) {
      console.log('Failed to save', document, 'to', filename);
      document.lastStatusCheckTime_ = new Date();
      document.obsolete = Document.Obsolete.UNKNOWN;
      return deferred.promise;
    });
  };

})();
