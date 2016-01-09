// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

class Element extends Node {
  /**
   * @param {!Document} document
   * @param {!NodeHandle} handle
   * @param {string} tagName
   * @param {string} id
   */
  constructor(document, handle, tagName, id) {
    super(document, handle);
    /** @const @type {string} id */
    this.id_ = id;
    /** @const @type {string} id */
    this.tagName_ = tagName;
  }

  /** @return {string} */
  get id() { return this.id_; }

  /** @return {string} */
  get tagName() { return this.tagName_; }

  /** @override @return {string} */
  get nodeNmae() { return this.tagName; }

  /** @return {string} */
  toString() {
    return `#{Element ${this.tagName_}}`;
  }
}
