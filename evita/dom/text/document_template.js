// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'text', function($export) {
  /** @type {!Map.<string|function(!Document):string>} */
  const templateMap = new Map();

  class DocumentTemplates {
    constructor() {
      Document.addObserver(this.didChangeDocuments.bind(this));
    }

    /**
     * @param {string} type
     * @param {!Document} document
     */
    didChangeDocuments(type, document) {
      if (type !== 'new')
        return;
      this.didNewDocument(document);
    }

    /**
     * @param {!Document} document
     */
    didNewDocument(document) {
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

  Object.defineProperty(DocumentTemplates, 'instance', {
    value: new DocumentTemplates()
  });

  $export({DocumentTemplates});
});
