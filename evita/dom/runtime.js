// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/**
 * @param {string} format_text
 * @param {dict}
 * @return {string}
 */
function localizeText(format_text, variable_map) {
  if (!(variable_map instanceof Object))
    return format_text;
  var text = format_text;
  Object.keys(variable_map).forEach(function(key) {
    text = text.replace('__' + key + '__', variable_map[key]);
  });
  return text;
}
