// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('modes');

goog.require('highlights.HighlightEngines');

goog.scope(function() {

const HighlightEngine = highlights.HighlightEngine;
const HighlightEngines = highlights.HighlightEngines;

/** @const @type {string} */
const kDefaultModeId = 'plain';

/** @const @type {!Map<string, ExtensionDescription>} */
const staticExtensionMap = new Map();

/** @const @type {!Map<string, string>} */
const staticFileNameMap = new Map();

/** @const @type {!Map<string, !ModeDescription>} */
const staticIdMap = new Map();

/**
 * @param {!TextDocument} document
 * @param {string} name
 * @return {string}
 */
function getProperty(document, name) {
  const value = document.properties.get(name);
  if (!value)
    return '';
  return /** @type {string} */ (value);
}

/**
 * @param {string} modeId
 * @return {!Mode}
 */
function makeMode(modeId) {
  /** @const @type {ModeDescription} */
  const description = Mode.findMode(modeId);
  if (description)
    return new Mode(description);
  if (modeId === kDefaultModeId)
    throw new Error('No ${kDefaultModeId} mode');
  return makeMode(kDefaultModeId);
}

class ExtensionDescription {
  /**
   * @public
   * @param {string} id
   * @param {string} modeId
   * @param {string} text
   */
  constructor(id, modeId, text) {
    /** @const @type {string} */
    this.id_ = id;
    /** @const @type {string} */
    this.modeId_ = modeId;
    /** @const @type {string} */
    this.text_ = text;
  }

  /** @public @return {string} */
  get id() { return this.id_; }

  /** @public @return {string} */
  get modeId() { return this.modeId_; }

  /** @public @return {string} */
  get text() { return this.text_; }

  /** @public @return {string} */
  toString() {
    return `ExtensionDescription('${this.id_}', '${this.modeId}',` +
        `'{$this.text_}')`;
  }
}

class ModeDescription {
  /**
   * @public
   * @param {string} id
   * @param {string} name
   */
  constructor(id, name) {
    /** @const @type {string} */
    this.id_ = id;
    /** @const @type {string} */
    this.name_ = name;
  }

  /** @public @return {string} */
  get id() { return this.id_; }

  /** @public @return {string} */
  get name() { return this.name_; }

  /** @public @return {string} */
  toString() { return `ModeDescription('${this.id_}', '{$this.name_}')`; }
}

class Mode {
  /**
   * @public
   * @param {!ModeDescription} description
   */
  constructor(description) {
    /** @const @type {!ModeDescription} */
    this.description_ = description;
    /** @type {TextDocument} */
    this.document_ = null;
    /** @type {HighlightEngine} */
    this.highlightEngine_ = null;
    /** @type {!Map} */
    this.keymap_ = new Map();
  }

  /**
   * @public
   * @param {!TextDocument} document
   */
  attach(document) {
    if (this.document_)
      throw new Error(this + ' is already attached to ' + this.document_);
    this.document_ = document;
    this.highlightEngine_ = HighlightEngines.createEngine(this.id, document);
  }

  /**
   * @public
   */
  detach() {
    if (!this.document_)
      throw new Error(this + ' is already attached to ' + this.document_);
    this.highlightEngine_.detach();
  }

  /**
   * @public
   * @param {!TextDocument} document
   * @param {number} hint
   */
  doColor(document, hint) {
    if (!this.document_)
      throw new Error(this + ' is detached.');
    this.highlightEngine_.doColor(hint);
  }

  /** @public @return {!ModeDescription} */
  get description() { return this.description_; }
  /** @public @return {TextDocument} */
  get document() { return this.document_; }
  /** @public @return {string} */
  get id() { return this.description_.id; }
  /** @public @return {!Map} */
  get keymap() { return this.keymap_; }
  /** @public @return {string} */
  get name() { return this.description_.name; }

  /**
   * @return {string}
   */
  toString() {
    if (this.document_)
      return `Mode('${this.id}', document: '${this.document_.name}')`;
    return `Mode('${this.id}')`;
  }

  /**
   * @param {!TextDocument} document
   * @return {!Mode}
   */
  static chooseMode(document) {
    /** @const @type {string} */
    const modeId =
        getProperty(document, 'mode') || getProperty(document, 'Mode');
    if (modeId) {
      /** @const @type {?ModeDescription} */
      const modeDescription = Mode.findMode(modeId.toLowerCase());
      if (modeDescription)
        return new Mode(modeDescription);
    }
    if (document.fileName !== '')
      return Mode.chooseModeByFileName(document.fileName);
    return Mode.chooseModeByFileName(document.name);
  }

  /**
   * @param {string} fileName
   * @return {!Mode}
   */
  static chooseModeByFileName(fileName) {
    const matches = /[.]([^.]+)$/.exec(FilePath.basename(fileName));
    if (!matches) {
      /** @type {string} */
      const modeId =
          staticFileNameMap.get(FilePath.basename(fileName)) || kDefaultModeId;
      return makeMode(modeId);
    }
    /** @const @type {string} */
    const extension = matches[1]
        /** @const @type {ExtensionDescription} */
        const description = staticExtensionMap.get(extension) || null;
    if (!description)
      return makeMode(kDefaultModeId);
    return makeMode(description.modeId);
  }

  /**
   * @param {string} id
   * @return {ModeDescription}
   */
  static findMode(id) { return staticIdMap.get(id) || null; }

  /**
   * @param {string} id
   * @param {string} modeId
   * @param {string} text
   */
  static registerExtension(id, modeId, text) {
    staticExtensionMap.set(id, new ExtensionDescription(id, modeId, text));
  }

  /**
   * @param {string} fileName
   * @param {string} modeId
   */
  static registerFileName(fileName, modeId) {
    staticFileNameMap.set(fileName, modeId);
  }

  /**
   * @param {string} id
   * @param {string} name
   */
  static registerMode(id, name) {
    /** @const @type {!ModeDescription} */
    const description = new ModeDescription(id, name);
    staticIdMap.set(id, description);
  }

  /**
   * @param {string} id
   * @param {string} aliasId
   */
  static registerModeAlias(id, name, aliasId) {
    staticIdMap.set(id, new ModeDescription(aliasId, name));
  }
}

/** @constructor */
modes.Mode = Mode;
});

/** @typedef {modes.Mode} */
var Mode;

global['Mode'] = modes.Mode;
