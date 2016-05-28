// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('base.Logger');

goog.scope(function() {
/*
 * Simple logger
 */
class Logger {
  /** @protected */
  constructor() {
    /** @type {number} */
    this.verbose_ = 0;
  }

  /** @protected */
  didChangeVerbose() {}

  /** @public @return {number} */
  get verbose() { return this.verbose_; }

  /** @public @param {number} level */
  set verbose(level) {
    if (this.verbose_ === level)
      return;
    this.verbose_ = level;
    this.didChangeVerbose();
  }

  /**
   * @protected
   * @param {number} level
   * @param {...*} args
   */
  log(level, ...args) {
    if (level >= this.verbose_)
      return;
    console.log(this, ...args);
  }
}

/** @constructor */
base.Logger = Logger;
});
