// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @typedef {string|function(!TextDocument):string} */
var TextDocumentTemplate;

(function() {
  /** @const @type {!Map.<string, !TextDocumentTemplate>} */
  const templateMap = new Map();

  /**
   * @param {string} extension
   * @param {!TextDocumentTemplate} template
   */
  function addTemplate(extension, template) {
    templateMap.set(extension, template);
  }

  /**
   * @param {!TextDocument} document
   */
  function applyTemplate(document) {
    const matches = (new RegExp('[.](.+)$')).exec(document.name);
    if (matches === null)
      return;
    const template = templateMap.get(matches[1]);
    if (!template)
      return;
    if (typeof(template) === 'string')
      return document.replace(0, 0, template);
    if (typeof(template) === 'function')
      return document.replace(0, 0, template(document));
  }

  /**
   * @param {string} extension
   */
  function removeTemplate(extension) { templateMap.delete(extension); }

  Object.defineProperties(TextDocument, {
    addTemplate: {value: addTemplate},
    applyTemplate: {value: applyTemplate},
    removeTemplate: {value: removeTemplate},
  });
})();
