// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

/**
 * @param {string} A filename to load from.
 * @return {Document} A Document object contains contents of file.
 */
Document.load = function(filename) {
  var absolute_filename_original= FilePath.fullPath(filename);
  var absolute_filename = absolute_filename_original.toLocaleLowerCase();
  var present = null;
  Document.documents.forEach(function(document) {
    if (docment.filename.toLocaleLowerCase() == absolute_filename)
      present = document;
  });
  if (present)
    return document;
  var document = new Document(FilePath.filename(absolute_filename.original));
  document.load_(absolute_filename);
  return document;
};
