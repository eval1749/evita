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


var parseKeyCombination = (function() {
  /** @const */ var MOD_CTRL = 0x200;
  /** @const */ var MOD_SHIFT = 0x400;

  var KEY_COMBINATION_RULES = [
    {
      modifiers: MOD_CTRL | MOD_SHIFT,
      re: /^Ctrl[-+]Shift[-+](.+)$/i
    },
    {
      modifiers: MOD_CTRL | MOD_SHIFT,
      re: /^Shift[-+]Ctrl[-+](.+)$/i
    },
    {
      modifiers: MOD_CTRL,
      re: /^Ctrl[-+](.+)$/i
    },
    {
      modifiers: MOD_SHIFT,
      re: /^Shift[-+](.+)$/i
    },
    {
      modifiers: 0,
      re: /^(.+)$/
    }
  ];

  /**
   * @param {string} spec.
   * @return {number} key code.
   */
  return function(spec) {
    var code = 0;
    KEY_COMBINATION_RULES.forEach(function(rule) {
      if (code)
        return;
      var matches = rule.re.exec(spec);
      if (!matches)
        return;
      var key_name = matches[1];
      var key_code = KEY_CODE_MAP[key_name.toLowerCase()];
      if (!key_code)
        throw new Error('Invalid key name: ' + key_name);
      code = rule.modifiers | key_code;
    });
    if (!code)
      throw new Error('Invalid key specification: ' + spec);
    return code;
  }
})();

