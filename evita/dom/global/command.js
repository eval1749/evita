// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('commands');

goog.scope(() => {

class Command {
  /**
   * @public
   * @param {string} name
   * @param {function(number=)} procedure
   * @param {string} description
   */
  constructor(name, procedure, description) {
    /** @const @type {string} */
    this.description_ = description;
    /** @const @type {string} */
    this.name_ = name;
    /** @const @type {function(number=)} */
    this.procedure_ = procedure;
  }

  /** @public @return {string} */
  get description() { return this.description_; }

  /** @public @return {string} */
  get name() { return this.name_; }

  /**
   * @public
   * @param {!Window} window
   * @param {number=} opt_count
   */
  execute(window, opt_count) {
    if (opt_count === undefined)
      return this.procedure_.call(window);
    this.procedure_.call(window, opt_count);
  }

  /** @override */
  toString() { return `Command(${this.name})`; }
}

/**
 * @this {!Window}
 * @param {number=} opt_count
 */
function unexpected(opt_count) {
  throw new Error('Keymap is not executalbe');
}

class Keymap extends Command {
  /**
   * @public
   * @param {string} name
   * @param {string} description
   */
  constructor(name, description) {
    super(name, unexpected, description);
    /** @const @type {!Map<number, !Command>} */
    this.map_ = new Map();
  }

  /**
   * @public
   * @param {number} keyCode
   * @return {?Command}
   */
  get(keyCode) { return this.map_.get(keyCode) || null; }

  /**
   * @public
   * @param {number} keyCode
   * @param {!Command} command
   */
  set(keyCode, command) { this.map_.set(keyCode, command); }

  /** @override */
  toString() { return `Keymap(${this.name} ${this.map_.size})`; }
};

/** @constructor */
commands.Command = Command;

/** @constructor */
commands.Keymap = Keymap;
});
