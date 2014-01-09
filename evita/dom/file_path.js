// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

/**
 * @param {string} A pathname.
 * @return {string} A filename part of pathname.
 */
FilePath.basename = function(path) {
  var last_slash = path.lastIndexOf('/');
  if (last_slash >= 0)
    return path.substr(last_slash + 1);
  //var last_back_slash = path.lastIndexOf(String.fromCharCode(0x5C));
  var last_back_slash = path.lastIndexOf('\\');
  if (last_back_slash >= 0)
    return path.substr(last_back_slash + 1);
  return path;
};

