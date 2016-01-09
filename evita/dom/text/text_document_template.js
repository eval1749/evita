// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'text', function($export) {
  /** @type {!Map.<string|function(!TextDocument):string>} */
  const templateMap = new Map();

  class TextDocumentTemplates {
    constructor() {
      TextDocument.addObserver(this.didChangeTextDocuments.bind(this));
    }

    /**
     * @param {string} type
     * @param {!TextDocument} document
     */
    didChangeTextDocuments(type, document) {
      if (type !== 'new')
        return;
      this.didNewTextDocument(document);
    }

    /**
     * @param {!TextDocument} document
     */
    didNewTextDocument(document) {
      if (document.length > 0)
        return;
      const matches = (new RegExp('[.](.+)$')).exec(document.name);
      if (matches === null)
        return;
      const template = templateMap.get(matches[1]);
      if (!template)
        return;
      const range = new Range(document);
      if (typeof(template) === 'string') {
        range.text = template;
        return;
      }
      if (typeof(template) === 'function') {
        range.text = template(document);
        return;
      }
    }

    static addTemplate(extension, templateText) {
      templateMap.set(extension, templateText);
    }

    static removeTemplate(extension) {
      templateMap.delete(extension);
    }
  }

  Object.defineProperty(TextDocumentTemplates, 'instance', {
    value: new TextDocumentTemplates()
  });

  $export({TextDocumentTemplates});
});
