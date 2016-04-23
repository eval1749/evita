// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights');

goog.scope(function() {

const HighlightEngine = highlights.HighlightEngine;

/** @typedef {function(!TextDocument):!HighlightEngine} */
var HighlightEngineCreator;

/** @const @type {!Map<string, !HighlightEngineCreator>} */
const staticEngineMap = new Map();

/** @const @type {!Map<string, !Set<string>>} */
const staticKeywordsMap = new Map();

class HighlightEngines {
  constructor() { throw new Error('HighlightEngines is static only class.'); }

  /**
   * @param {string} name
   * @return {?HighlightEngineCreator}
   */
  static engineFor(name) { return staticEngineMap.get(name) || null; }

  /**
   * @param {string} name
   * @return {!Set<string>}
   */
  static keywordsFor(name) {
    const keywords = staticKeywordsMap.get(name);
    if (!keywords)
      throw new Error(`No keywords for '${name}'`);
    return keywords;
  }

  /**
   * @param {string} name
   * @param {!HighlightEngineCreator} creator
   */
  static registerEngine(name, creator, keywords) {
    staticEngineMap.set(name, creator);
    staticKeywordsMap.set(name, keywords);
  }
}

/** @constructor */
highlights.HighlightEngines = HighlightEngines;
});
