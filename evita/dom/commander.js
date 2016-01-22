// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

global.commander =
   /**
    * @type {{
    *  handleEvent: function(!Event),
    *  startArgument: function(),
    *  startQuote: function()
    * }}
    */
    (Object.create(/**@type {!Object}*/(Object.prototype), (function() {
  /** @enum{string} */
  var State = {
    ARGUMENT: 'ARGUMENT',
    NORMAL: 'NORMAL',
    QUOTE: 'QUOTE',
    START: 'START'
  };

  var argument =
    /**
     * @type {{
     *    handleEvent: function(!KeyboardEvent): boolean,
     *    hasValue: boolean,
     *    number: number,
     *    numDigits: number,
     *    reset: function(),
     *    value: number
     *  }}
     */
      (Object.create(/**@type {!Object}*/(Object.prototype), {
    handleEvent: {
      /** @type {function(!KeyboardEvent): boolean} */
      value: function(event) {
        var code = event.keyCode;
        if (code == 0x2D) {
          if (this.sign_ || this.numDigits_)
            return false;
          this.sign_ = -1;
          this.hasValue = true;
          return true;
        }
        if (code >= 0x30 && code <= 0x39) {
          if (this.numDigits_) {
            this.number_ *= 10;
            this.number_ += code - 0x30;
          } else {
            this.number_ = code - 0x30;
            this.sign_ = 1;
          }
          ++this.numDigits_;
          this.hasValue = true;
          return true;
        }
        return false;
      }
    },
    hasValue: { value: false, writable: true },
    number: { value: 0, writable: true },
    numDigits: { value: 0, writable: true },
    reset: {
      /** @type {function()} */
      value: function() {
        this.hasValue = false;
        this.number_ = 0;
        this.numDigits_ = 0;
        this.sign_ = 0;
      }
    },
    /** @private*/ sign: { value: 0, writable: true },
    value: {
      /** @type {function():number} */
      get: function() {
        if (!this.hasValue)
          return 1;
        if (this.numDigits_)
          return this.number_ * this.sign_;
        return this.sign_ ? -4 : 4;
      }
    }
  }));

  /** @type {!function(!KeyboardEvent)} */
  var handler;

  /** @type {?Keymap} */
  var keymap;

  /** @type {Array.<number>} */
  var key_codes = [];

  /**
   * @param {!Event} event
   * @return {?Window}
   */
  function eventWindow(event) {
    return event.target instanceof Window ?
        /** @type {!Window} */(event.target) : Editor.activeWindow();
  }

  /**
   * @param {!KeyboardEvent} event
   * @return {!Function|!Keymap|undefined}
   */
  function getKeyBinding(event) {
    /**
     * @param {?Keymap} keymap
     * @param {number} code
     */
    function get(keymap, code) {
      return keymap && keymap instanceof Map ? keymap.get(code) : undefined;
    }

    var code = event.keyCode;
    var present = get(keymap, code);
    if (present)
      return present;
    if (event.target instanceof TextWindow) {
      var text_window = /**@type {TextWindow} */(event.target);
      present = get(text_window.document['keymap'], code);
      if (present)
        return present;
    }
    present = get(event.target['keymap'], code);
    if (present)
      return present;
    for (var runner = event.target; runner && runner;
         runner = Object.getPrototypeOf(runner)) {
      present = get(runner.constructor['keymap'], code);
      if (present)
        return present;
    }
    return undefined;
  }

  /** @param {!Event} event */
  function handleEvent(event) {
    if (event instanceof KeyboardEvent)
      handleKeyboardEvent(/**@type{!KeyboardEvent}*/(event));
  }

  /** @param {!KeyboardEvent} event */
  function handleEventAsArgument(event) {
    if (argument.handleEvent(event))
      return;
    handler = handleEventAsCommand;
    handleEventAsCommand(event);
  }

  /** @param {!KeyboardEvent} event */
  function handleEventAsCommand(event) {
    key_codes.push(event.keyCode);
    var binding = getKeyBinding(event);
    if (!binding) {
      handleUnboundKeySequence(event);
      return;
    }

    if (typeof(binding) == 'function') {
      try {
        if (argument.hasValue)
          binding.call(event.target, argument.value);
        else
          binding.call(event.target);
      } catch (exception) {
        if (!(exception instanceof TextDocumentReadOnly))
          throw exception;
        if (!(event.target instanceof TextWindow))
          throw exception;
        var window = /** @type{!TextWindow} */ (event.target);
        var error = /** @type{!TextDocumentReadOnly} */(exception);
        if (error.document != window.document)
          throw error;
        window.status = 'Can not change readonly document.';
      }
      if (handler == handleEventAsCommand)
        reset();
      return;
    }

    if (binding instanceof Map) {
      keymap = /** @type {!Keymap} */(binding);
      return;
    }

    handleUnboundKeySequence(event);
  }

  /** @param {!KeyboardEvent} event */
  function handleEventAsQuote(event) {
    var char_code = event.keyCode;
    if (char_code >= 0x240 && char_code <= 0x25F)
      char_code &= 0x1F;
    if (char_code >= 0x80) {
      Editor.messageBox(eventWindow(event),
        'We don\'t support non-ASCII code insert: ' + char_code.toString(16),
        MessageBox.ICONWARNING);
      reset();
      return;
    }
    var window = /** @type {TextWindow} */(event.target);
    var range = window.selection.range;
    range.text = String.fromCharCode(char_code).repeat(argument.value);
    range.collapseTo(range.end);
    reset();
  }

  /** @param {!KeyboardEvent} event */
  function handleKeyboardEvent(event) {
    if (event.type != 'keydown')
      return;
    handler(event);
  }

  /** @param {!KeyboardEvent} event */
  function handleUnboundKeySequence(event) {
    var key_seq = key_codes.map(unparseKeyCode).join(' ');
    Editor.messageBox(eventWindow(event),
        'Unbound key sequence: ' + key_seq,
        MessageBox.ICONWARNING);
    reset();
  }

  function reset() {
    argument.reset();
    handler = handleEventAsCommand;
    key_codes = [];
    keymap = null;
  }

  function startArgument() {
    argument.hasValue = true;
    handler = handleEventAsArgument;
  }

  function startQuote() {
    handler = handleEventAsQuote;
  }

  /**
   * @param {number} key_code
   * @return {string}
   */
  function unparseKeyCode(key_code) {
    /** @const @type {number} */ var MOD_CTRL = 0x200;
    /** @const @type {number} */ var MOD_SHIFT = 0x400;
    return ((key_code & MOD_CTRL) ? 'Ctrl+' : '') +
           ((key_code & MOD_SHIFT) ? 'Shift+' : '') +
           KEY_NAMES[key_code & 0x1FF];
  }

  reset();

  return {
    handleEvent: { value: handleEvent },
    startArgument: { value: startArgument },
    startQuote: { value: startQuote }
  };
})()));
