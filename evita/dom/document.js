// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

/**
 * @param {string} A filename to load from.
 * @return {Document} A Document object contains contents of file.
 */
Document.load = function(filename) {
  var absolute_filename = FilePath.fullPath(filename);
  var canonical_filename = absolute_filename.toLocaleLowerCase();
  var present = Document.list.find(function(document) {
    return document.filename.toLocaleLowerCase() == canonical_filename;
  });
  if (present)
    return present;
  var document = new Document(FilePath.basename(filename));
  document.load_(absolute_filename);
  return document;
};
