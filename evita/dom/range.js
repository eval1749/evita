// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

/**
 * @this {Range}
 */
Range.prototype.toLocaleLowerCase = function() {
  this.text = this.text.toLocaleLowerCase();
};

/**
 * @this {Range}
 */
Range.prototype.toLocaleUpperCase = function() {
  this.text = this.text.toLocaleUpperCase();
};

/**
 * @this {Range}
 */
Range.prototype.toLowerCase = function() {
  this.text = this.text.toLowerCase();
};

/**
 * @this {Range}
 */
Range.prototype.toUpperCase = function() {
  this.text = this.text.toUpperCase();
};
