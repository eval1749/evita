// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.HtmlLexer = (function(options) {
  /**
   * @constructor
   * @extends {XmlLexer}
   * @param {!Document} document
   */
  function HtmlLexer(document) {
    this.scriptLexer_ = new JavaScriptLexer(document, this);
    XmlLexer.call(this, document, options);
  }

  HtmlLexer.prototype = Object.create(XmlLexer.prototype, {
    constructor: {value: HtmlLexer},
  });

  return HtmlLexer;
})({
  keywords: []
});
