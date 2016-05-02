// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @implements {TextDocumentSetObserver}
 */
var SimpleTextDocumentSetObserver = function();

/** @param {!TextDocument} document */
SimpleTextDocumentSetObserver.prototype.didAddTextDocument = function(document) {};

/** @param {!TextDocument} document */
SimpleTextDocumentSetObserver.prototype.didRemoveTextDocument = function(document) {};
