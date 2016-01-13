// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function NewRegExp(source, options) {
  return new RegExp(source, options)
}

if (!Array.prototype.values)
  Array.prototype.values = Array.prototype[Symbol.iterator];
