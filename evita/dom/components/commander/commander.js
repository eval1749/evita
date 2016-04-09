// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('components.commander');

(function() {
  /** @const @type {number} */ const MOD_ALT = 0x800;
  /** @const @type {number} */ const MOD_CTRL = 0x200;
  /** @const @type {number} */ const MOD_SHIFT = 0x400;

  /** @const @type {string} */ const PROPERTY_KEYMAP = 'keymap';

  /** @enum {string} */
  const State = {
    ARGUMENT: 'ARGUMENT',
    COMMAND: 'COMMAND',
    QUOTE: 'QUOTE',
  };

  //////////////////////////////////////////////////////////////////////
  //
  // Argument
  //
  class Argument {
    constructor() {
      /** @type {boolean} */
      this.hasValue_ = false;
      /** @type {number} */
      this.numDigits_ = 0;
      /** @type {number} */
      this.number_ = 0;
      /** @type {number} */
      this.sign_ = 0;
    }

    /** @return {boolean} */
    get hasValue() { return this.hasValue_; }

    /** @return {number} */
    get value() {
      if (!this.hasValue_)
        return 1;
      if (this.numDigits_)
        return this.number_ * this.sign_;
      return this.sign_ ? -4 : 4;
    }

    /**
     * @public
     * @param {number} keyCode
     * @return {boolean}
     */
    feed(keyCode) {
      console.assert(this.hasValue_, this);
      if (keyCode === Unicode.HYPHEN_MINUS) {
        if (this.sign_ || this.numDigits_)
          return false;
        this.sign_ = -1;
        return true;
      }
      if (keyCode < Unicode.DIGIT_ZERO || keyCode > Unicode.DIGIT_NINE)
        return false;
      if (this.numDigits_ === 0) {
        this.number_ = keyCode - Unicode.DIGIT_ZERO;
        this.sign_ = 1;
      } else {
        this.number_ *= 10;
        this.number_ += keyCode - Unicode.DIGIT_ZERO;
      }
      ++this.numDigits_;
      return true;
    }

    /** @public */
    reset() {
      this.hasValue_ = false;
      this.number_ = 0;
      this.numDigits_ = 0;
      this.sign_ = 0;
    }

    /** @public */
    start() {
      this.reset();
      this.hasValue_ = true;
    }
  }

  /**
   * @param {!Window} window
   * @param {!Function} commandFunction
   * @param {!Argument} argument
   */
  function executeCommand(window, commandFunction, argument) {
    try {
      if (argument.hasValue)
        commandFunction.call(window, argument.value);
      else
        commandFunction.call(window);
    } catch (exception) {
      if (exception instanceof TextDocumentReadOnly) {
        reportReadOnly(window, exception);
        return;
      }
      throw exception;
    }
  }

  /**
   * @param {!Window} window
   * @param {number} keyCode
   * @param {number} repeatCount
   */
  function executeQuote(window, keyCode, repeatCount) {
    if (!(window instanceof TextWindow))
      return;
    // TODO(eval1749): We should use |TextDocument.prototype.splice()|
    // instead of |TextRange|.
    const textWindow = /** @type {TextWindow} */ (window);
    const range = textWindow.selection.range;
    range.text = String.fromCharCode(keyCode).repeat(repeatCount);
    range.collapseTo(range.end);
  }

  /**
   * @param {Keymap} currentKeymap
   * @param {!Window} window
   * @param {number} keyCode
   * @return {Function|Keymap}
   */
  function keyBindingOf(window, keyCode, currentKeymap) {
    for (const keymap of searchPathOf(currentKeymap, window)) {
      const present = keymap.get(keyCode) || null;
      if (present)
        return present;
    }
    return null;
  }

  /**
   * @param {Object} object
   * @return {Keymap}
   */
  function keymapFor(object) {
    if (object === null)
      return null;
    if (!(PROPERTY_KEYMAP in object))
      return null;
    const maybeKeymap = object[PROPERTY_KEYMAP];
    if (!(maybeKeymap instanceof Map))
      return null;
    return /** @type {!Keymap} */ (maybeKeymap);
  }

  /**
   * @param {number} keyCode
   * @return {number}
   * Strip Control modifier for Ctrl+@ to Ctrl+Z.
   */
  function convertKeyCodeToCharCode(keyCode) {
    if (keyCode >= 0x240 && keyCode <= 0x25F)
      return keyCode & 0x1F;
    return keyCode;
  }

  /**
   * @param {Window} window
   * @param {!TextDocumentReadOnly} error
   */
  function reportReadOnly(window, error) {
    if (!(window instanceof TextWindow))
      throw error;
    if (error.document !== window.document)
      throw error;
    window.status = 'Can not change readonly document.';
  }

  /**
   * @param {Keymap} currentKeymap
   * @param {!Window} window
   * @return {!Generator<!Keymap>}
   * We search key binding in following order:
   *  - document
   *  - window
   *  - window class
   *  - parent class
   */
  function * searchPathOf(currentKeymap, window) {
    if (currentKeymap)
      yield currentKeymap;

    // Document instance keymap
    if (window instanceof TextWindow) {
      const textWindow = /**@type {!TextWindow} */ (window);
      const documentKeymap = keymapFor(textWindow.document);
      if (documentKeymap)
        yield documentKeymap;
    }

    // Window instance keymap
    const windowKeymap = keymapFor(window);
    if (windowKeymap)
      yield windowKeymap;

    // Looking for class keymap
    for (let runner = window; runner !== null;
         runner = Object.getPrototypeOf(/** @type {!Object} */ (runner))) {
      const keymap = keymapFor(runner.constructor);
      if (keymap)
        yield keymap;
    }
  }

  /**
   * @param {number} keyCode
   * @return {string}
   */
  function unparseKeyCode(keyCode) {
    /** @const @type {!Array<string>} */ const keys = [];
    if (keyCode & MOD_CTRL)
      keys.push('Ctrl');
    if (keyCode & MOD_SHIFT)
      keys.push('Shift');
    if (keyCode & MOD_ALT)
      keys.push('Alt');
    if (keyCode >= 0x20 && keyCode <= 0x7E)
      keys.push(String.fromCharCode(keyCode));
    else
      keys.push(KEY_NAMES[keyCode & 0x1FF]);
    return keys.join('+');
  }

  //////////////////////////////////////////////////////////////////////
  //
  // Commander
  //
  class Commander {
    constructor() {
      /** @const @type {!Argument} */
      this.argument_ = new Argument();
      /** @type {!Array.<number>} */
      this.keyCodes_ = [];
      /** @type {?Keymap} */
      this.keymap_ = null;
      /** @type {State} */
      this.state_ = State.COMMAND;
    }

    /**
     * @public
     * @param {!Window} window
     * @param {number} keyCode
     */
    execute(window, keyCode) {
      switch (this.state_) {
        case State.ARGUMENT:
          return this.executeAsArgument(window, keyCode);
        case State.COMMAND:
          return this.executeAsCommand(window, keyCode);
        case State.QUOTE:
          return this.executeAsQuote(window, keyCode);
        default:
          throw new Error(`Invalid state: ${this.state_}`);
      }
    }

    /**
     * @private
     * @param {!Window} window
     * @param {number} keyCode
     */
    executeAsArgument(window, keyCode) {
      if (this.argument_.feed(keyCode))
        return;
      this.state_ = State.COMMAND;
      this.executeAsCommand(window, keyCode);
    }

    /**
     * @private
     * @param {!Window} window
     * @param {number} keyCode
     */
    executeAsCommand(window, keyCode) {
      this.keyCodes_.push(keyCode);
      const binding = keyBindingOf(window, keyCode, this.keymap_);
      if (binding === null)
        return this.reportUnboundKeySequence(window, keyCode);

      if (typeof(binding) === 'function') {
        executeCommand(window, binding, this.argument_);
        if (this.state_ === State.COMMAND)
          this.reset();
        return;
      }

      if (binding instanceof Map) {
        this.keymap_ = /** @type {!Keymap} */ (binding);
        return;
      }

      this.reportUnboundKeySequence(window, keyCode);
    }

    /**
     * @private
     * @param {!Window} window
     * @param {number} keyCode
     */
    executeAsQuote(window, keyCode) {
      const charCode = convertKeyCodeToCharCode(keyCode);
      if (charCode >= 0x80) {
        Editor.messageBox(
            window,
            'We don\'t support non-ASCII code insert: ' + charCode.toString(16),
            MessageBox.ICONWARNING);
        this.reset();
        return;
      }
      executeQuote(window, charCode, this.argument_.value);
      this.reset();
    }

    /**
     * @private
     * @param {!Window} window
     * @param {number} keyCode
     */
    reportUnboundKeySequence(window, keyCode) {
      const keySequence = this.keyCodes_.map(unparseKeyCode).join(' ');
      Editor.messageBox(
          window, `Unbound key sequence: ${keySequence}`,
          MessageBox.ICONWARNING);
      this.reset();
    }

    /** @private */
    reset() {
      this.argument_.reset();
      this.keyCodes_ = [];
      this.keymap_ = null;
      this.state_ = State.COMMAND;
    }

    /** @private */
    startArgument() {
      this.argument_.start();
      this.state_ = State.ARGUMENT;
    }

    /** @private */
    startQuote() { this.state_ = State.QUOTE; }

    /** @public static */
    static argumentCommand() { Commander.instance.startArgument(); }

    /** @public static @return {!Commander} */
    static get instance() {
      return /** @type {!Commander} */ (base.Singleton.get(Commander));
    }

    /** @public static */
    static quoteCommand() { Commander.instance.startQuote(); }
  }

  /** @const */
  components.commander.Commander = Commander;
})();
