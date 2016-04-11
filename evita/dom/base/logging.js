// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

/** @type {!Map<string, number>} */
const levelMap = new Map();

function disableFor(name) {
  if (name === '*')
    return levelMap.clear();
  levelMap.delete(name);
}

function enableFor(name, level) {
  levelMap.set(name, level);
}

function asStringLiteral(text) {
  const result = [];
  for (let index = 0; index < text.length; ++index) {
    const charCode = text.charCodeAt(index);
    if (charCode === 0x0A) {
      result.push('\\n');
      continue;
    }
    if (charCode < 0x20) {
      result.push(`\\u{${charCode.toString(16)}}`);
      continue;
    }
    result.push(String.fromCharCode(charCode));
  }
  return `'${result.join('')}'`;
}

function getLocation() {
  function getStack() {
    let stack;
    try {
      throw new Error('get line number');
    } catch (error) {
      stack = error.stack.split('\n').slice(1);
    }
    return stack
  }
  const locations = getStack();
  let index = locations.findIndex(line => line.indexOf('at DVLOG ') > 0);
  if (index > 0) {
    const pattern = new RegExp('at (.+?) \\((.+?):(\\d+):(\\d+)\\)$');
    const match = pattern.exec(locations[index + 1]);
    if (match)
      return {
        functionName: match[1],
        fileName: match[2],
        lineNumber: match[3],
        column: match[4]
      };
    console.log('Failed to locate', locations[index + 1]);
    return {functionName: '?', fileName: '?', lineNumber: 0, column: 0};
  }
  console.log('Failed to locate', index, locations);
  return {functionName: '?', fileName: '?', lineNumber: 0, column: 0};
}

function DVLOG(level, ...args) {
  const location = getLocation();
  const path = FilePath.split(location.fileName);
  const fileName = path.components[path.components.length - 1];
  const fileLevel = levelMap.has(fileName) ? levelMap.get(fileName) : -1;
  if (level > fileLevel)
    return;
  global.DVLOG.output(location, args);
}

Object.defineProperties(DVLOG, {
  disableFor: {value: disableFor},
  enableFor: {value: enableFor},
});

global.DVLOG = DVLOG;
base.asStringLiteral = asStringLiteral;
});
