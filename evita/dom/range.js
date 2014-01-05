// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

Range.prototype.toLocaleLowerCase = function() {
  this.text = this.text.toLocaleLowerCase();
};

Range.prototype.toLocaleUpperCase = function() {
  this.text = this.text.toLocaleUpperCase();
};

Range.prototype.toLowerCase = function() {
  this.text = this.text.toLowerCase();
};

Range.prototype.toUpperCase = function() {
  this.text = this.text.toUpperCase();
};
