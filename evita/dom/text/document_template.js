// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'text', function($export) {
  /** @type {!Map.<string|function(!Document):string>} */
  const templateMap = new Map();

  /**
   * @param {!DocumentEvent} event
   */
  function didAttachDocument(event) {
    const document = /** @type {!Document} */(event.target);
      document.removeEventListener(Event.Names.ATTACH, didAttachDocument);
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

  class DocumentTemplates {
    constructor() {
      Document.addObserver(this.didChangeDocuments.bind(this));
    }

    /**
     * @param {string} type
     * @param {!Document} document
     */
    didChangeDocuments(type, document) {
      if (type !== 'add')
        return;
      // TODO(eval1749): We should have another way to know new document
      // for file.
      document.addEventListener(Event.Names.ATTACH, didAttachDocument);
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
