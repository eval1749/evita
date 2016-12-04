// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

class ScriptModule {
  /**
   * @param {string} fullName
   */
  constructor(fullName) {
    /** @const @type {string} */
    this.fullName_ = fullName;
    /** @type {?NativeScriptModule} */
    this.handle_ = null;
  }

  /** @return {string} */
  get fullName() { return this.fullName_; }

  /** @return {!NativeScriptModule} */
  get handle() {
    if (!this.handle_)
      throw new Error(`${this} dose not have handle.`);
    return this.handle_;
  }

  /**
   * @param {!NativeScriptModule} handle
   */
  set handle(handle) {
    if (this.handle_)
      throw new Error(`${this} already has handle.`);
    this.handle_ = handle;
  }

  /** @override */
  toString() { return `SciprtModule("${this.fullName_}")`; }
}

/** @interface */
const ScriptTextProvider = function() {};

/**
 * @param {string} specifier
 * @return {!Promise<string>}
 */
ScriptTextProvider.prototype.computeFullName = function(specifier) {};

/**
 * @param {string} fullName
 * @return {string}
 */
ScriptTextProvider.prototype.dirNameOf = function(fullName) {};

/**
 * @param {string} specifier
 * @param {string} dirName
 * @return {string}
 */
ScriptTextProvider.prototype.normalizeSpecifier = function(specifier, dirName) {
};

/**
 * @param {string} fullName
 * @return {!Promise<string>}
 */
ScriptTextProvider.prototype.readScriptText = function(fullName) {};

const RE_BACKSLASH = new RegExp('\\\\', 'g');

/**
 * @implements {ScriptTextProvider}
 */
class PlatformScriptTextProvider {
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
   * @return {!Promise<string>}
   */
  async readScriptText(fullName) {
    /** @type {string} */
    const encoding = 'utf-8';
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
  }
}

/** @type {!ScriptTextProvider} */
const kPlatformScriptTextProvider = new PlatformScriptTextProvider();

class ScriptModuleLoader {
  /**
   * @param {!ScriptTextProvider} scriptTextProvider
   */
  constructor(scriptTextProvider) {
    /** @const @type {!Map<string, !ScriptModule>} */
    this.fullNameToModule_ = new Map();
    /** @const @type {!Map<NativeScriptModule, !ScriptModule>} */
    this.handleToModule_ = new Map();
    /** @const @type {!ScriptTextProvider} */
    this.scriptTextProvider_ = scriptTextProvider;
  }

  /**
   * @private
   * @param {string} specifier
   * @param {string} dirName
   * @return {string}
   */
  normalizeSpecifier(specifier, dirName) {
    return this.scriptTextProvider_.normalizeSpecifier(specifier, dirName);
  }

  /**
   * @private
   * @param {string} fullName
   * @return string
   */
  dirNameOf(fullName) { return this.scriptTextProvider_.dirNameOf(fullName); }

  /**
   * @private
   * @param {string} fullName
   * @return {!Promise<!ScriptModule>}
   */
  async fetchModleTree(fullName) {
    /** @type {?ScriptModule} */
    const present = this.fullNameToModule_.get(fullName) || null;
    if (present)
      return present;
    const module = new ScriptModule(fullName);
    this.fullNameToModule_.set(fullName, module);

    /** @type {string} */
    const scriptText = await this.readScriptText(fullName);

    /** @type {!NativeScriptModule} */
    const handle = NativeScriptModule.compile(fullName, scriptText);
    module.handle = handle;
    this.handleToModule_.set(handle, module);

    /** @type {string} */
    const dirName = this.dirNameOf(fullName);
    /** @type {!Array<!ScriptModule>} */
    const requestModules = [];
    for (const request of handle.requests) {
      const requestFullName = this.normalizeSpecifier(request, dirName);
      requestModules.push(this.fetchModleTree(requestFullName));
    }

    await Promise.all(requestModules);
    return module;
  }

  /**
   * @param {!NativeScriptModule} handle
   * @return {!ScriptModule}
   */
  fromHandle(handle) {
    const module = this.handleToModule_.get(handle);
    if (!module)
      throw new Error(`No assocaited ScriptModule for ${handle}`);
    return module;
  }

  /**
   * @public
   * @param {string} specifier
   */
  async load(specifier) {
    const fullName = await this.scriptTextProvider_.computeFullName(specifier);
    const module = await this.fetchModleTree(fullName);
    module.handle.instantiate(this.resolveCallback.bind(this));

    const result = module.handle.evaluate();
    // TODO(eval1749): We would like to known |Module::Evaluate()| returns
    // other than |undefined|.
    if (result !== undefined)
      throw new Error(`${module} returns ${result}.`);
    return result;
  }

  /**
   * @private
   * @param {string} fullName
   * @return {!Promise<string>}
   */
  readScriptText(fullName) {
    return this.scriptTextProvider_.readScriptText(fullName);
  }

  /**
   * @private
   * @param {string} specifier
   * @param {!NativeScriptModule} refererHandle
   * @return {!NativeScriptModule}
   */
  resolveCallback(specifier, refererHandle) {
    /** @type {!ScriptModule} */
    const referer = this.fromHandle(refererHandle);
    /** @type {string} */
    const dirName = this.dirNameOf(referer.fullName);
    /** @type {string} */
    const fullName = this.normalizeSpecifier(specifier, dirName);
    /** @type {?ScriptModule} */
    const module = this.fullNameToModule_.get(fullName) || null;
    if (!module)
      throw new Error(`No such module ${fullName}`);
    return module.handle;
  }
}

// export { ScriptModule, loadModule };
global.ScriptModule = ScriptModule;
global.ScriptModuleLoader = ScriptModuleLoader;
global.ScriptTextProvider = ScriptTextProvider;

});
