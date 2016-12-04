// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('engine');
goog.provide('engine.ScriptModuleLoader');

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

  /** @return {boolean} */
  get isCompiled() { return this.handle_ !== null; }

  /** @override */
  toString() { return `SciprtModule("${this.fullName_}")`; }
}

class Session {
  constructor() {
    /** @const @type {!Array<!ScriptModule>} */
    this.modules_ = [];
  }

  /** @return {!Array<!ScriptModule>} */
  get modules() { return this.modules_; }

  /**
   * @param {!ScriptModule} module
   */
  addModule(module) { this.modules_.push(module); }
}

// |ScriptModuleLoader| provides following methods for loading and unloading
// ES6 modules:
//  - load(fullName)
//  - unload(fullName)
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
   * @param {!Session} session
   * @param {string} fullName
   * @param {string} referrer
   * @return {!Promise<!ScriptModule>}
   */
  async fetchModleTree(session, fullName, referrer) {
    /** @type {?ScriptModule} */
    const present = this.fullNameToModule_.get(fullName) || null;
    if (present)
      return present;
    const module = new ScriptModule(fullName);
    this.fullNameToModule_.set(fullName, module);
    session.addModule(module);

    try {
      /** @type {string} */
      const scriptText = await this.readScriptText(fullName, referrer);

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
        requestModules.push(
            this.fetchModleTree(session, requestFullName, fullName));
      }

      await Promise.all(requestModules);
      return module;
    } catch (exception) {
      this.unloadModule(module);
      throw exception;
    }
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
   * @return {!Promise<!Array<string>>} A list of module full name.
   */
  async load(specifier) {
    const fullName = await this.scriptTextProvider_.computeFullName(specifier);
    const present = this.fullNameToModule_.get(fullName) || null;
    if (present)
      this.unloadModule(present);
    const session = new Session();
    const module = await this.fetchModleTree(session, fullName, '-');
    module.handle.instantiate(this.resolveCallback.bind(this));
    const result = module.handle.evaluate();
    // TODO(eval1749): We would like to known |Module::Evaluate()| returns
    // other than |undefined|.
    if (result !== undefined)
      throw new Error(`${module} returns ${result}.`);
    return session.modules.map(module => module.fullName);
  }

  /**
   * @private
   * @param {string} fullName
   * @param {string} referrer
   * @return {!Promise<string>}
   */
  readScriptText(fullName, referrer) {
    return this.scriptTextProvider_.readScriptText(fullName, referrer);
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

  /**
   * @public
   * @param {string} specifier
   */
  async unload(specifier) {
    const fullName = await this.scriptTextProvider_.computeFullName(specifier);
    this.unloadModule(fullName);
  }

  /**
   * @private
   * @param {!ScriptModule} module
   */
  unloadModule(module) {
    this.fullNameToModule_.delete(module.fullName);
    if (!module.isCompiled)
      return true;
    this.handleToModule_.delete(module.handle);
    return true;
  }
}

/** @constructor */
engine.ScriptModuleLoader = ScriptModuleLoader;

});
