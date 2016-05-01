// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

class TextDocumentError extends Error {
  /**
   * @param {!TextDocument} document
   * @param {string=} opt_message
   */
  constructor(document, opt_message) {
    super(opt_message);
    this.document = document;
    this.name = this.constructor.name;
  }
}

class TextDocumentNotReady extends TextDocumentError {
  /**
   * @param {!TextDocument} document
   * @param {string=} opt_message
   */
  constructor(document, opt_message) { super(document, opt_message); }
}

class TextDocumentReadOnly extends TextDocumentError {
  /**
   * @param {!TextDocument} document
   * @param {string=} opt_message
   */
  constructor(document, opt_message) { super(document, opt_message); }
}

/** @constructor */
text.TextDocumentError = TextDocumentError;

/** @constructor */
text.TextDocumentNotReady = TextDocumentNotReady;

/** @constructor */
text.TextDocumentReadOnly = TextDocumentReadOnly;
});

// TODO(eval1749): We should not export |TextDocumentError|,
// |TextDocumentNotReady|, and |TextDocumentReadOnly| to |global|.
/** @constructor */
var TextDocumentError = text.TextDocumentError;

/** @constructor */
var TextDocumentNotReady = text.TextDocumentNotReady;

/** @constructor */
var TextDocumentReadOnly = text.TextDocumentReadOnly;
