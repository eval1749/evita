// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.HtmlLexer = (function(options) {
  class HtmlLexer extends global.XmlLexer {
    /**
     * @param {!Document} document
     */
    constructor(document) {
      super(document, options);
    }
  }
  // TODO(eval1749): Once closure compiler support |static get|, we should use
  // it.
  Object.defineProperty(HtmlLexer, 'keywords', {
    get: function() { return options.keywords; }
  });
  return HtmlLexer;
})({
  childLexers: {
    script: JavaScriptLexer
  },
  ignoreCase: true,
  keywords: []
});
