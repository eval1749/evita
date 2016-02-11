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
    const range = new TextRange(document);
    if (typeof(template) === 'string') {
      range.text = template;
      return;
    }
    if (typeof(template) === 'function') {
      range.text = template(document);
      return;
    }
  }

  /**
   * @param {string} extension
   */
  function removeTemplate(extension) {
    templateMap.delete(extension);
  }

  Object.defineProperties(TextDocument, {
    addTemplate: {value: addTemplate},
    applyTemplate: {value: applyTemplate},
    removeTemplate: {value: removeTemplate},
  });
})();
