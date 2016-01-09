// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.TextDocumentError = (function() {
  /**
   * @param {!TextDocument} document
   * @param {string=} opt_message
   */
  function TextDocumentError(document, opt_message) {
    if (arguments.length >= 2) {
      Error.call(this, opt_message);
      // TODO(eval1749): Not sure why |Error.call(this, opt_message)| doesn't set
      // |message| property.
      this.message = opt_message;
    } else {
      Error.call(this);
    }
    this.document = document;
  }

  TextDocumentError.prototype = Object.create(
    /** @type {!Object} */ (Error.prototype), {
      document: {configurable: false, enumerable: false, writable: true}
  });
  TextDocumentError.prototype.constructor = TextDocumentError;
  TextDocumentError.prototype.name = TextDocumentError.name;
  return TextDocumentError;
})();


global.TextDocumentNotReady = (function() {
  /**
   * @param {!TextDocument} document
   * @param {string=} opt_message
   */
  function TextDocumentNotReady(document, opt_message) {
    TextDocumentError.apply(this, arguments);
  }
  TextDocumentNotReady.prototype = /** @type {!TextDocumentNotReady} */ (Object.create(
    /** @type {!Object} */ (TextDocumentError.prototype), {}));
  TextDocumentNotReady.prototype.constructor = TextDocumentNotReady;
  TextDocumentNotReady.prototype.name = TextDocumentNotReady.name;
  return TextDocumentNotReady;
})();

global.TextDocumentReadOnly = (function() {
  /**
   * @param {!TextDocument} document
   * @param {string=} opt_message
   */
  function TextDocumentReadOnly(document, opt_message) {
    TextDocumentError.apply(this, arguments);
  }
  TextDocumentReadOnly.prototype = /** @type {!TextDocumentReadOnly} */(Object.create(
    /** @type {!Object} */ (TextDocumentError.prototype), {}));
  TextDocumentReadOnly.prototype.constructor = TextDocumentReadOnly;
  TextDocumentReadOnly.prototype.name = TextDocumentReadOnly.name;
  return TextDocumentReadOnly;
})();
