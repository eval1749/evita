// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

class VerboseSettings {
  constructor() {
    /** @type {number} */
    this.level_ = -1;

    /** @type {!Map<string, number>} */
    this.levelMap_ = new Map();
  }

  /** @public @return {number} */
  get level() { return this.level_; }

  /** @public @param {number} newLevel */
  set level(newLevel) { this.level_ = newLevel; }

  /** @public @param {string} name*/
  disableFor(name) {
    if (name === '*')
      return this.levelMap_.clear();
    this.levelMap_.delete(name);
  }

  /**
   * @public
   * @param {string} name
   * @param {number} level
   */
  enableFor(name, level = 0) { this.levelMap_.set(name, level); }

  /**
   * @public
   * @param {string} name
   * @return {number}
   */
  levelFor(name) {
    const level = this.levelMap_.get(name);
    return typeof(level) === 'number' ? level : -1;
  }
}

/** @const @type {!VerboseSettings} */
const settings = new VerboseSettings();

/**
 * @param {string} text
 * @return {string}
 */
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

/**
 * @param {number} level
 * @param {...*} args
 */
function DVLOG(level, ...args) {
  if (level > settings.level)
    return;
  const location = getLocation();
  const path = FilePath.split(location.fileName);
  const fileName = path.components[path.components.length - 1];
  if (level > settings.levelFor(fileName))
    return;
  global.DVLOG.output(location, args);
}

Object.defineProperties(DVLOG, {
  disableFor: {value: settings.disableFor.bind(settings)},
  enableFor: {value: settings.enableFor.bind(settings)},
  level: {get: () => settings.level, set: x => settings.level = x},
});

global.DVLOG = DVLOG;
base.asStringLiteral = asStringLiteral;
});
