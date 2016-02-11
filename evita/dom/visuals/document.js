// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('visuals.Node');

class Document extends Node {
  constructor() { super(null, NodeHandle.createDocument()); }

  /**
   * @param {string} tagName
   * @param {string=} id
   * @return {!Element}
   */
  createElement(tagName, id = '') {
    const handle = NodeHandle.createElement(this.handle_, tagName, id);
    return new Element(this, handle, tagName, id);
  }

  /**
   * @param {!Uint8Array} data
   * @return {!Shape}
   */
  createShape(data) {
    const handle = NodeHandle.createShape(this.handle_, data);
    return new Shape(this, handle, data);
  }

  /**
   * @param {string} data
   * @return {!Text}
   */
  createText(data) {
    const handle = NodeHandle.createText(this.handle_, data);
    return new Text(this, handle, data);
  }

  /** @override @return {string} */
  get nodeName() { return '#document'; }
}
