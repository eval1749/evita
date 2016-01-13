// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//////////////////////////////////////////////////////////////////////
//
// DOMTokenList
//
class DOMTokenList {
  /** @param {!DOMTokenListOwner} owner */
  constructor(owner) {
    /** @private @type {Array<string>} */
    this.cachedList_ = null;

    /** @private @const @type {!Array<string>} */
    this.list_ = [];

    /** @private @const {!DOMTokenListOwner} */
    this.owner_ = owner;
  }

  /** @return {number} */
  get length() { return this.list_.length; }

  /** @return {!Generator<string>} */
  [Symbol.iterator]() { return this.list_.values(); }

  /**
   * @param {string} token
   */
  add(token) {
    if (this.contains(token))
      return;
    this.list_.push(token);
    this.clearListCache();
    this.owner_.didAddToken(token);
  }

  /** @private */
  clearListCache() {
    this.cachedList_ = null;
  }

  /**
   * @param {string} token
   * @return {boolean}
   */
  contains(token) { return this.findIndex_(token) >= 0; }

  /**
   * @param {string} token
   * @return {number}
   */
  findIndex_(token) {
    return this.list_.findIndex(present => present === token);
  }

  /**
   * @param {number} index
   * @return {string}
   */
  item(index) { return this.list_[index]; }

  /**
   * @param {string} token
   */
  remove(token) {
    const index = this.findIndex_(token);
    if (index < 0)
      return;
    this.list_.splice(index, 1);
    this.owner_.didRemoveToken(token);
    this.clearListCache();
  }

  /**
   * @param {string} token
   */
  toggle(token) {
    const index = this.findIndex_(token);
    if (index < 0) {
      this.list_.push(token);
      this.owner_.didAddToken(token);
    } else {
      this.list_.splice(index, 1);
      this.owner_.didRemoveToken(token);
    }
    this.clearListCache();
  }
}
