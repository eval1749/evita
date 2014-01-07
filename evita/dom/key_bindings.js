// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
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

  function parseKeyCombination(spec) {
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

  editor.setKeyBinding = function(combination, command) {
    editor.setKeyBinding_(parseKeyCombination(combination), command);
  };
})();

(function() {
  // Select all contents
  editor.setKeyBinding('Ctrl+A', function() {
    var range = this.selection.range;
    range.start = 0
    range.end = range.document.length;
    this.selection.startIsActive = false;
  });
})();
