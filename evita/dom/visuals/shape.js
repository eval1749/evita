// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

class Shape extends Node {
  /**
   * @param {!Document} document
   * @param {!NodeHandle} handle
   * @param {!Uint8Array} data
   */
  constructor(document, handle, data) {
    super(document, handle);
    this.data_ = data;
  }

  /** @return {!Uint8Array} */
  get data() { return this.data_; }

  /** @param {!Uint8Array} newData */
  set data(newData) {
    if (this.data_ === newData)
      return;
    this.data_ = newData;
    NodeHandle.setShapeData(this.handle_, newData);
  }

  /** @override @return {string} */
  get nodeName() { return '#shape'; }
}
