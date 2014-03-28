// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @param {number} offset */
global.TextFieldSelection.prototype.collapseTo = function(offset) {
  this.anchorOffset = this.focusOffset = offset;
};

/** @param {number} offset */
global.TextFieldSelection.prototype.extendTo = function(offset) {
  this.focusOffset = offset;
};
