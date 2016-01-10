// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 */
global.TextStyle = (function () {
  return function TextStyle() {
  };
})();

global.TextStyle.prototype = Object.create(
    /** @type {Object} */(TextStyle.prototype), {
  backgroundColor: {
    enumerable: true,
    writable: true
  },
  color: {
    enumerable: true,
    writable: true
  },
  fontFamily: {
    enumerable: true,
    writable: true
  },
  fontSize: {
    enumerable: true,
    writable: true
  },
  fontWeight: {
    enumerable: true,
    writable: true
  },
  syntax: {
    enumerable: true,
    writable: true
  },
  textDecoration: {
    enumerable: true,
    writable: true
  },
});
