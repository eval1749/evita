// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!Map.prototype.keys) {
  Map.prototype.keys = function() {
    var keys = [];
    this.forEach(function(value, key) {
      keys.push(key);
    });
    return keys;
  };
}

if (!Set.prototype.keys) {
  Set.prototype.keys = function() {
    var keys = [];
    this.forEach(function(value) {
      keys.push(value);
    });
    return keys;
  };
}
