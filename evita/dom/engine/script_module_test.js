// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('engine.ScriptModuleLoader');

goog.scope(function() {

const ScriptModuleLoader = engine.ScriptModuleLoader;

/** @type {string} */
const kSampleDirName = '/samples';

/** @type {!Map<string, string>} */
const kSampleTextMap = new Map();

// simple
kSampleTextMap.set('simple_root.js', [
  'import {simple1} from "simple1.js";',
  'import {simple2} from "simple2.js";',
  'testing.test("SimpleModuleTest", t => {',
  '  t.expect(simple1).toEqual(123);',
  '  t.expect(simple2).toEqual(456);',
  '});',
].join('\n'));
kSampleTextMap.set('simple1.js', 'export let simple1 = 123;');
kSampleTextMap.set('simple2.js', 'export let simple2 = 456;');

// chain
kSampleTextMap.set('chain_root.js', [
  'import {chain1} from "chain1.js";',
  'testing.test("ChainModuleTest", t => {',
  '  t.expect(chain1).toEqual(456);',
  '});',
].join('\n'));
kSampleTextMap.set('chain1.js', [
  'import {chain2} from "chain2.js";',
  'export let chain1 = chain2;',
].join('\n'));
kSampleTextMap.set('chain2.js', 'export let chain2 = 456;');

// cycle
// cycle1 =
//   #1=#{Module cycle2: #{Module cycle3: #{Module cycle1: #1#, value: 123}}}
kSampleTextMap.set('cycle_root.js', [
  'import * as cycle1 from "cycle1.js";',
  'testing.test("CycleModuleTest", t => {',
  '  t.expect(cycle1.cycle2.cycle3.value).toEqual(123);',
  '});',
].join('\n'));
kSampleTextMap.set('cycle1.js', [
  'import * as cycle2 from "cycle2.js";',
  'export {cycle2};',
].join('\n'));
kSampleTextMap.set('cycle2.js', [
  'import * as cycle3 from "cycle3.js";',
  'export {cycle3};',
].join('\n'));
kSampleTextMap.set('cycle3.js', [
  'import * as cycle1 from "cycle1.js";',
  'export let value = 123;',
  'export {cycle1};',
].join('\n'));


// loop
kSampleTextMap.set('loop_root.js', [
  'import * as loop1 from "loop1.js";',
  'testing.test("LoopModuleTest", t => {',
  '  t.expect(loop1.value).toEqual(123);',
  '});',
].join('\n'));
kSampleTextMap.set('loop1.js', 'export * from "loop2.js";');
kSampleTextMap.set('loop2.js', 'export * from "loop3.js";');
kSampleTextMap.set('loop3.js', [
  'import "loop1.js";',
  'export const value = 123;',
].join('\n'));

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
  dirNameOf(fullName) { return kSampleDirName; }

  /**
   * @param {string} specifier
   * @param {string} dirName
   * @return {string}
   */
  normalizeSpecifier(specifier, dirName) {
    if (dirName !== kSampleDirName)
      throw new Error(`dirName "${dirName} must be "${kSampleDirName}".`);
    return `${specifier}`;
  }

  /**
   * @param {string} fullName
   * @param {string} referer
   * @return {!Promise<string>}
   */
  readScriptText(fullName, referer) {
    const text = kSampleTextMap.get(fullName);
    if (text === undefined)
      throw new Error(`No such sample "${fullName} in ${referer}".`);
    return Promise.resolve(text);
  }
}

/** @type {!ScriptTextProvider} */
const kScriptTextProvider = new SampleScriptTextProvider();

function testModules(specifier) {
  const loader = new ScriptModuleLoader(kScriptTextProvider);
  loader.load(specifier);
}

testModules('simple_root.js');
testModules('chain_root.js');
testModules('cycle_root.js');
testModules('loop_root.js');

});
