// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

/** @enum {string} */
const State = {
  COMPILED: 'COMPILED',
  COMPILING: 'COMPILEING',
  EVALUATED: 'EVALUATED',
  EVALUATING: 'EVALUATING',
  FAILED: 'FAILED',
  INSTANTIATED: 'INSTANTIATED',
  INSTANTIATING: 'INSTANTIATING',
  LOADED: 'LOADED',
  LOADING: 'LOADING',
  NOT_STARTED: 'NOT_STARTED',
  RESOLVED: 'RESOLVED',
  RESOLVING: 'RESOLVING',
};

/** @type {!Map<State, State>} */
const kCanAdvanceMap = (() => {
  /** @type {!Map<State, State>} */
  const map = new Map();
  [State.NOT_STARTED, State.LOADING, State.LOADED, State.COMPILING,
   State.COMPILED, State.RESOLVING, State.RESOLVED, State.INSTANTIATING,
   State.INSTANTIATED, State.EVALUATING, State.EVALUATED]
      .reduce((previous, current) => {
        map.set(previous, current);
        return current;
      });
  return map;
})();

/**
 * @param {State} oldState
 * @param {State} newState
 * @return {boolean}
 */
function canAdvanceTo(oldState, newState) {
  /** @type {State} */
  const allowed = kCanAdvanceMap.get(oldState) || State.FAILED;
  if (allowed === State.FAILED)
    throw new Error(`Invalid state ${oldState}`);
  return newState === allowed;
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
    this.state_ = State.NOT_STARTED;
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

  /** @return {boolean} */
  get isStarted() { return this.state_ !== State.NOT_STARTED; }

  /** @return {!Promise} */
  get promise() { return this.promise_; }

  /**
   * @param {State} newState
   */
  advanceTo(newState) {
    if (!canAdvanceTo(this.state_, newState))
      throw new Error(`${this} can not advance to ${newState}.`);
    this.state_ = newState;
  }

  /**
   * @public
   * @param {!Error} error
   */
  fail(error) {
    this.state_ = State.FAILED;
    this.reject_(error);
  }

  /**
   * @param {*}
   */
  finish(result) { this.resolve_(result); }

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
    /** @const @type {!Map<string, !ScriptModule} */
    this.fullNameToModule_ = new Map();
    /** @const @type {!Map<NativeScriptModule, !ScriptModule} */
    this.handleToModule_ = new Map();
    /** @const @type {!ScriptTextProvider} */
    this.scriptTextProvider_ = scriptTextProvider;
  }

  /**
   * @private
   * @param {!ScriptModule} module
   * @param {string} scriptText
   * @return {!Array<!ScriptModule>} A list of requested module.
   */
  compile(module, scriptText) {
    /** @type {!NativeScriptModule} */
    const handle = NativeScriptModule.compile(module.fullName, scriptText);
    module.handle = handle;
    this.handleToModule_.set(handle, module);
    /** @type {string} */
    const dirName = this.dirNameOf(module.fullName);
    return handle.requests.map(specifier => {
      /** @type {string} */
      const requestFullName = this.computeFullName(specifier, dirName);
      return this.getOrCreateModule(requestFullName);
    });
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
   * @param {string} fullName
   * @return string
   */
  dirNameOf(fullName) { return this.scriptTextProvider_.dirNameOf(fullName); }

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
   * @private
   * @param {string} fullName
   * @return {!ScriptModuole}
   */
  getOrCreateModule(fullName) {
    const module = this.fullNameToModule_.get(fullName);
    if (module)
      return module;
    const newModule = new ScriptModule(fullName);
    this.fullNameToModule_.set(fullName, newModule);
    return newModule;
  }

  /**
   * @public
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
    if (module.isStarted)
      return module.Promise;
    try {
      module.advanceTo(State.LOADING);
      const scriptText = await this.readScriptText(module.fullName);
      module.advanceTo(State.LOADED);

      module.advanceTo(State.COMPILING);
      /** @type {!Array<!ScriptModule>} */
      const requests = this.compile(module, scriptText);
      module.advanceTo(State.COMPILED);

      module.advanceTo(State.RESOLVING);
      await this.loadRequests(requests);
      module.advanceTo(State.RESOLVED);

      module.advanceTo(State.INSTANTIATING);
      module.handle.instantiate(this.resolveCallback.bind(this));
      module.advanceTo(State.INSTANTIATED);

      module.advanceTo(State.EVALUATING);
      const result = module.handle.evaluate();
      module.advanceTo(State.EVALUATED);

      // TODO(eval1749): We would like to known |Module::Evaluate()| returns
      // other than |undefined|.
      if (result !== undefined)
        throw new Error(`${module} returns ${result}.`);

      module.finish(result);
    } catch (error) {
      module.fail(error);
    }
    return module.promise;
  }

  /**
   * @param {!Array<!ScriptModule>} requests
   * @reutrn {!Promise}
   */
  loadRequests(requests) {
    return Promise.all(requests.map(request => this.loadInternal(request)));
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
    const requestFullName = this.computeFullName(specifier, referer.fullName);
    /** @type {!ScriptModule} */
    const requestModule = this.getOrCreateModule(requestFullName);
    return requestModule.handle;
  }
}

// export { ScriptModule, loadModule };
global.ScriptModule = ScriptModule;
global.ScriptModuleLoader = ScriptModuleLoader;
global.ScriptTextProvider = ScriptTextProvider;

});
