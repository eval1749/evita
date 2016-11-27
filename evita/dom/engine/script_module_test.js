// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

/** @type {!Map<string, string>} */
const kSampleTextMap = new Map();

// simple
kSampleTextMap.set('simple_root.js', [
  'import {simple1} from "simple1.js";',
  'import {simple2} from "simple2.js";',
  'console.log("simple.js", `simple1=${simple1}`, `simple2=${simple2}`);',
].join('\n'));
kSampleTextMap.set('simple1.js', 'export let simple1 = 123;');
kSampleTextMap.set('simple2.js', 'export let simple2 = 456;');

// chain
kSampleTextMap.set('chain_root.js', [
  'import {chain1} from "chain1.js";',
  'console.log("chain_root.js", `chain1=${chain1}`);',
].join('\n'));
kSampleTextMap.set('chain1.js', [
  'import {chain2} from "chain2.js";',
  'export let chain1 = chain2;',
].join('\n'));
kSampleTextMap.set('chain2.js', 'export let chain2 = 456;');

// cycle
kSampleTextMap.set('cycle_root.js', 'import {cycle1} from "cycle1.js";');
kSampleTextMap.set('cycle1.js', 'import {cycle2} from "cycle2.js";');
kSampleTextMap.set('cycle2.js', 'import {cycle3} from "cycle3.js";');
kSampleTextMap.set('cycle3.js', 'import {cycle1} from "cycle1.js";');

class SampleScriptTextProvider {
  /**
   * @param {string} specifier
   * @return {!Promise<string>}
   */
  computeFullName(specifier) { return specifier; }

  /**
   * @param {string} fullName
   * @return {string}
   */
  dirNameOf(fullName) { '/root' }

  /**
   * @param {string} specifier
   * @param {string} dirName
   * @return {string}
   */
  normalizeSpecifier(specifier, dirName) { return `${dirName}/${specifier}`; }

  /**
   * @param {string} fullName
   * @return {!Promise<string>}
   */
  readScriptText(fullName) {
    const text = kSampleTextMap.get(fullName);
    if (text === undefined)
      throw new Error(`No such sample "${fullName}".`);
    return Promise.resolve(text);
  }
}

/** @type {!ScriptTextProvider} */
const kScriptTextProvider = new SampleScriptTextProvider();

function loadModule(specifier) {
  const loader = new ScriptModuleLoader(kScriptTextProvider);
  return loader.load(specifier);
}

loadModule('simple_root.js');
loadModule('chain_root.js');
loadModule('cycle_root.js');

});
