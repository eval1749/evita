// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

/** @enum {string} */
const State = {
  COMPILED: 'COMPILED',
  EVALUATING: 'EVALUATING',
  FAILED: 'FAILED',
  FINISHED: 'FINISHED',
  INITIALIZED: 'INITIALIZED',
  INSTANTIATED: 'INSTANTIATED',
  LOADING: 'LOADING',
  LOADED: 'LOADED',
  RESOLVED: 'RESOLVED',
};

/**
 * @param {State} oldState
 * @param {State} newState
 * @return {boolean}
 */
function canAdvanceTo(oldState, newState) {
  switch (oldState) {
    case State.COMPILED:
      return newState === State.RESOLVED;
    case State.EVALUATING:
      return newState === State.FINISHED;
    case State.FINISHED:
      return false;
    case State.INITIALIZED:
      return newState === State.LOADING;
    case State.INSTANTIATED:
      return newState === State.EVALUATING;
    case State.LOADING:
      return newState === State.LOADED;
    case State.LOADED:
      return newState === State.COMPILED;
    case State.RESOLVED:
      return newState === State.INSTANTIATED;
  }
  throw new Error(`Invalid state ${oldState}`);
}

// |ScriptModuleMap| provides
//  1. mapping from module full name to |ScriptModule|.
//  2. mapping from |NativeScriptModule| to |ScriptModule|; used for
//     resolve referer in callback of |NativeScriptModule#instantiate()|.
class ScriptModuleMap {
  constructor() {
    /** @const @type {!Map<string, !ScriptModule} */
    this.fullNameToModule_ = new Map();
    /** @const @type {!Map<NativeScriptModule, !ScriptModule} */
    this.handleToModule_ = new Map();
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
   * @param {string} fullName
   * @return {!ScriptModuole}
   */
  getOrCreate(fullName) {
    console.assert(typeof(fullName) === 'string', fullName);
    const module = this.fullNameToModule_.get(fullName);
    if (module)
      return module;
    const newModule = new ScriptModule(fullName);
    this.fullNameToModule_.set(fullName, newModule);
    return newModule;
  }

  /**
   * @param {!ScriptModule} module
   */
  registerHandle(module) {
    console.assert(module.handle, module);
    console.assert(module.state === State.COMPILED, module);
    this.handleToModule_.set(module.handle, module);
  }
}

class ScriptModule {
  /**
   * @param {string} fullName
   */
  constructor(fullName) {
    /** @const @type {string} */
    this.fullName_ = fullName;
    /** @type {?NativeScriptModule} */
    this.handle_ = null;
    /** @const @type {!Promise} */
    this.promise_ = new Promise((resolve, reject) => {
      /** @const @type {!function(boolean)} */
      this.resolve_ = resolve;
      /** @const @type {!function(!Error)} */
      this.reject_ = reject;
    });
    /** @type {State} */
    this.state_ = State.INITIALIZED;
  }

  /** @return {string} */
  get fullName() { return this.fullName_; }

  /** @return {!NativeScriptModule} */
  get handle() { return this.handle_; }

  /** @return {!Promise} */
  get promise() { return this.promise_; }

  /** @return {State} */
  get state() { return this.state_; }

  /**
   * @param {State} newState
   */
  advanceTo(newState) {
    console.log('  advanceTo', this, newState);
    console.assert(canAdvanceTo(this.state_, newState), this, newState);
    this.state_ = newState;
  }

  /**
   * @param {string} scriptText
   */
  compile(scriptText) {
    console.assert(canAdvanceTo(this.state_, State.COMPILED), this);
    if (this.handle_)
      throw new Error(`${module} is already compiled.`);
    this.handle_ = NativeScriptModule.compile(this.fullName, scriptText);
    this.advanceTo(State.COMPILED);
  }

  /**
   * @param {!Error} error
   */
  fail(error) {
    console.log('  fail', this, error);
    this.handle_ = null;
    this.state_ = State.FAILED;
    this.reject_(error);
  }

  finish() {
    this.advanceTo(State.EVALUATING);
    this.handle_.evaluate();
    this.advanceTo(State.FINISHED);
    this.resolve_(this);
  }

  /** @return {boolean} */
  isFinished() { return this.state_ === State.FINISHED; }

  /** @override */
  toString() { return `SciprtModule("${this.fullName_}", ${this.state_})`; }
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
   * @param {string} specifier
   * @return {!Promise<string>}
   */
  computeFullName(specifierIn) {
    const specifier = this.normalize(specifierIn);
    return Os.File.computeFullPathName(specifier);
  }

  /**
   * ScriptTextProvider#dirNameOf
   * @param {string} fullName
   * @return {string}
   */
  dirNameOf(fullName) { return FilePath.dirName(fullName); }

  /**
   * @param {string} specifier
   * @return {string}
   */
  normalize(specifier) { return specifier.replace(RE_BACKSLASH, '/'); }

  /**
   * ScriptTextProvider#normalizeSpecifier
   * @param {string} specifierIn
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
    const file = await Os.File.open(scriptPath);
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
    /** @const @type {!ScriptModuleMap} */
    this.moduleMap_ = new ScriptModuleMap();
    /** @const @type {!ScriptTextProvider} */
    this.scriptTextProvider_ = scriptTextProvider;
  }

  /**
   * @private
   * @param {string} specifier
   * @param {string} dirName
   * @return {string}
   */
  computeFullName(specifier, dirName) {
    return this.scriptTextProvider_.computeFullName(specifier, dirName);
  }

  /**
   * @private
   * @param {!ScriptModule} module
   * @return {!Array<!ScriptModule>}
   */
  computeRequests(module) {
    /** @type {string} */
    const dirName = this.dirNameOf(module.fullName);
    return module.handle.requests.map(specifier => {
      /** @type {string} */
      const requestFullName = this.computeFullName(specifier, dirName);
      /** @type {!ScriptModule} */
      return this.getOrCreateModule(requestFullName);
    });
  }

  /**
   * @private
   * @param {string} fullName
   * @return string
   */
  dirNameOf(fullName) { return this.scriptTextProvider_.dirNameOf(fullName); }

  /**
   * @param {!NativeScriptModule} handle
   * @return {!ScriptModule}
   */
  fromHandle(handle) { return this.moduleMap_.fromHandle(handle); }

  /**
   * @private
   * @param {string} fullName
   * @return {!ScriptModuole}
   */
  getOrCreateModule(fullName) { return this.moduleMap_.getOrCreate(fullName); }

  /**
   * @param {string} specifier
   * @param {ScriptTextProvider=} provider
   */
  async load(specifier) {
    const fullName = await this.computeFullName(specifier);
    const module = this.getOrCreateModule(fullName);
    return this.loadInternal(module);
  }

  /**
   * @private
   * @param {!ScriptModule} module
   * @return {!Promise}
   */
  async loadInternal(module) {
    console.log('  loadInternal', module);
    module.advanceTo(State.LOADING);
    const scriptText = await this.readScriptText(module.fullName);
    module.advanceTo(State.LOADED);
    try {
      module.compile(scriptText);
      this.moduleMap_.registerHandle(module);
      /** @type {!Array<!ScriptModule>} */
      const requests = this.computeRequests(module);
      // TODO(eval1749): We should check cyclic dependency of |requests| and
      // |module|.

      // Wait all requests are fnished or one of them is rejected.
      await Promise.all(requests.map(request => this.loadIfNeeded(request)));
      module.advanceTo(State.RESOLVED);

      // All requested modules for |module| are loaded, we tell |module|
      // about them.
      module.handle.instantiate((specifier, refererHandle) => {
        const referer = this.fromHandle(refererHandle);
        const requestFullName =
            this.computeFullName(specifier, referer.fullName);
        const requestModule = this.getOrCreateModule(requestFullName);
        if (!requestModule.isFinished())
          throw new Error(`${requestModule} should be finished.`);
        return requestModule.handle;
      });
      module.advanceTo(State.INSTANTIATED);

      // Execute script in |module|.
      module.finish();
    } catch (error) {
      module.fail(error);
    }
    return module.promise;
  }

  /**
   * @private
   * @param {!ScriptModule} module
   * @return {!Promise}
   */
  loadIfNeeded(module) {
    if (module.state_ === State.INITIALIZED)
      return this.loadInternal(module);
    return module.promise;
  }

  /**
   * @private
   * @param {string} fullName
   * @return {!Promise<string>}
   */
  readScriptText(fullName) {
    return this.scriptTextProvider_.readScriptText(fullName);
  }
}

// export { ScriptModule, loadModule };
global.ScriptModule = ScriptModule;
global.ScriptModuleLoader = ScriptModuleLoader;
global.ScriptTextProvider = ScriptTextProvider;

});
