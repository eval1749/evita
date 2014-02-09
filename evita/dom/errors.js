// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.DocumentError = (function() {
  /**
   * @param {!Document} document
   * @param {string=} opt_message
   */
  function DocumentError(document, opt_message) {
    if (arguments.length >= 2) {
      Error.call(this, opt_message);
      // TODO(yosi) Not sure why |Error.call(this, opt_message)| doesn't set
      // |message| property.
      this.message = opt_message;
    } else {
      Error.call(this);
    }
    this.document = document;
  }

  DocumentError.prototype = Object.create(
    /** @type {!Object} */ (Error.prototype), {
      document: {configurable: false, enumerable: false, writable: true}
  });
  DocumentError.prototype.constructor = DocumentError;
  DocumentError.prototype.name = DocumentError.name;
  return DocumentError;
})();


global.DocumentNotReady = (function() {
  /**
   * @param {!Document} document
   * @param {string=} opt_message
   */
  function DocumentNotReady(document, opt_message) {
    DocumentError.apply(this, arguments);
  }
  DocumentNotReady.prototype = /** @type {!DocumentNotReady} */ (Object.create(
    /** @type {!Object} */ (DocumentError.prototype), {}));
  DocumentNotReady.prototype.constructor = DocumentNotReady;
  DocumentNotReady.prototype.name = DocumentNotReady.name;
  return DocumentNotReady;
})();

global.DocumentReadOnly = (function() {
  /**
   * @param {!Document} document
   * @param {string=} opt_message
   */
  function DocumentReadOnly(document, opt_message) {
    DocumentError.apply(this, arguments);
  }
  DocumentReadOnly.prototype = /** @type {!DocumentReadOnly} */(Object.create(
    /** @type {!Object} */ (DocumentError.prototype), {}));
  DocumentReadOnly.prototype.constructor = DocumentReadOnly;
  DocumentReadOnly.prototype.name = DocumentReadOnly.name;
  return DocumentReadOnly;
})();
