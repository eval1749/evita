// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('visuals.Text');

goog.scope(function() {

/** @constructor */
const Document = visuals.Document;

/** @constructor */
const Node = visuals.Node;

class Text extends Node {
  /**
   * @param {!Document} document
   * @param {!NodeHandle} handle
   * @param {string} data
   */
  constructor(document, handle, data) {
    super(document, handle);
    this.data_ = data;
  }

  /** @return {string} */
  get data() { return this.data_; }

  /** @param {string} newData */
  set data(newData) {
    if (this.data_ === newData)
      return;
    this.data_ = newData;
    NodeHandle.setTextData(this.handle_, newData);
  }

  /** @override @return {string} */
  get nodeName() { return '#text'; }
}

/** @constructor */
visuals.Text = Text;
});
