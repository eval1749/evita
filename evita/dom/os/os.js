// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

/**
 * @const @type {string}
 */
Os.environmentStrings;

/** @const @type {!Map<string, string>} */
const environment = new Map();

// Os.environmentStrings has "key1=val1\0key2=val2\0...keyN=valN".
// It has "=C:", "=D:", ... "=Z:" for working directory of each drive,
// "=ExitCode=000000".
Os.environmentStrings.split('\0').forEach(function(keyValueString) {
  const match = keyValueString.match("^(.[^=]+)=(.*)$");
  if (!match)
    return;
  environment.set(match[1], match[2]);
});

/**
 * @param {string} name
 * @return {string|undefined}
 */
function getenv(name) {
  return environment.get(name);
}


Os.getenv = getenv;
});
