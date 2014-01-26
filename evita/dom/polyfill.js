// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

Map.prototype.delete = (function() {
  var original_delete = Map.prototype.delete;
  return function(key) {
    if (this.keys_) {
      this.keys_ = this.keys_.filter(function(element) {
        return element !== key;
      });
    }
    original_delete.call(this, key);
  };
})();

Map.prototype.keys = function() {
  return this.keys_;
};

Map.prototype.set = (function() {
  var original_set = Map.prototype.set;
  return function (key, value) {
    if (!this.keys_)
      this.keys_ = [key];
    else
      this.keys_.push(key);
    original_set.call(this, key, value);
  };
})();

Set.prototype.delete = (function() {
  var original_delete = Set.prototype.delete;
  return function(key) {
    if (this.keys_) {
      this.keys_ = this.keys_.filter(function(element) {
        return element !== key;
      });
    }
    original_delete.call(this, key);
  };
})();

Set.prototype.keys = function() {
  return this.keys_;
};

Set.prototype.set = (function() {
  var original_set = Set.prototype.set;
  return function (key, value) {
    if (!this.keys_)
      this.keys_ = [key];
    else
      this.keys_.push(key);
    original_set.call(this, key, value);
  };
})();
