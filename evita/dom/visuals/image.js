// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

class Image extends Node {
  /**
   * @param {!Document} document
   * @param {!NodeHandle} handle
   * @param {!ImageData} data
   */
  constructor(document, handle, data) {
    super(document, handle);
    this.data_ = data;
  }

  /** @return {!ImageData} */
  get data() { return this.data_; }

  /** @param {!ImageData} newData */
  set data(newData) {
    if (this.data_ === newData)
      return;
    this.data_ = newData;
    NodeHandle.setImageData(this.handle_, newData);
  }

  /** @override @return {string} */
  get nodeName() { return '#image'; }
}
