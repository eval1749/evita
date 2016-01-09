// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

  /** @override @return {string} */
  get nodeNmae() { return '#text'; }
}
