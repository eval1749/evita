// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('engine.ScriptModuleLoader');

goog.scope(function() {

const ScriptModuleLoader = engine.ScriptModuleLoader;

const LINE_COMMENT = '\x2F/';
const RE_BACKSLASH = new RegExp('\\\\', 'g');

/**
 * @implements {ScriptTextProvider}
 */
class FileScriptTextProvider {
  constructor() {}

  /**
   * ScriptTextProvider#computeFullName
   * @param {string} specifierIn
   * @return {!Promise<string>}
   */
  computeFullName(specifierIn) {
    /** @type {string} */
    const specifier = this.normalize(specifierIn);
    return Os.File.computeFullPathName(specifier);
  }

  /**
   * ScriptTextProvider#dirNameOf
   * @param {string} fullName
   * @return {string}
   */
  dirNameOf(fullName) { return FilePath.dirname(fullName); }

  /**
   * @param {string} specifier
   * @return {string}
   */
  normalize(specifier) { return specifier.replace(RE_BACKSLASH, '/'); }

  /**
   * ScriptTextProvider#normalizeSpecifier
   * @param {string} specifier
   * @param {string} dirName
   * @return {string}
   */
  normalizeSpecifier(specifier, dirName) {
    const split = FilePath.split(specifier);
    if (split.absolute)
      return FilePath.normalize(specifier);
    return FilePath.normalize(`${dirName}/${specifier}`);
  }

  /**
   * @param {string} fullName
   * @param {string} referrer
   * @return {!Promise<string>}
   */
  async readScriptText(fullName, referrer) {
    /** @type {string} */
    const encoding = 'utf-8';
    try {
      /** @type {Os.File} */
      const file = await Os.File.open(fullName);
      try {
        /** @type {!Array<string>} */
        const texts = [];
        /** @type {!ArrayBufferView} */
        const buffer = new Uint8Array(4096);
        /** @type {!TextDecoder} */
        const decoder = new TextDecoder(encoding, {fatal: true});
        for (;;) {
          /** @const @type {number} */
          const numRead = await file.read(buffer);
          if (numRead === 0)
            break;
          texts.push(decoder.decode(buffer.subarray(0, numRead)));
        }
        return texts.join('').replace(/\r\n/g, '\n');
      } finally {
        file.close();
      }
    } catch (error) {
      switch (error['winLastError']) {
        case 2:
          throw new Error(`No such file "${fullName}" in "${referrer}"`);
        case 3:
          throw new Error(`No such path "${fullName}" in "${referrer}"`);
      }
      throw error;
    }
  }
}

/** @type {!ScriptModuleLoader} */
const loader = new ScriptModuleLoader(new FileScriptTextProvider());

/**
 * @param {string} scriptPath
 * @param {!Object=} options
 * @return {!Promise<!Array<string>>}
 *
 * Options:
 *  encoding: string
 *  verbose: boolean
 */
async function load(scriptPath, options = {}) {
  if (options.verbose)
    console.log(LINE_COMMENT, 'loading', scriptPath);
  const result = loader.load(scriptPath);
  if (options.verbose)
    console.log(LINE_COMMENT, 'loaded', scriptPath, result);
  return result;
}

/**
 * @param {string} scriptPath
 * @param {!Object=} opt_options
 * @return {!Promise}
 */
repl.load = load;
});

global.load = repl.load;
