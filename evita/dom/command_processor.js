// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

global.CommandProcessor =
   /**
    * @type{{
    *  handleEvent: function(!KeyboardEvent),
    *  startArgument: function(),
    *  startQuote: function()
    * }}
    */
    (Object.create(/**@type{!Object}*/(Object.prototype), (function() {
  /** @enum{string} */
  var State = {
    ARGUMENT: 'ARGUMENT',
    NORMAL: 'NORMAL',
    QUOTE: 'QUOTE',
    START: 'START'
  };

  var argument =
    /**
     * @type{{
     *    handleEvent: function(!KeyboardEvent): boolean,
     *    hasValue: boolean,
     *    number: number,
     *    numDigits: number,
     *    reset: function(),
     *    value: number
     *  }}
     */
      (Object.create(/**@type{!Object}*/(Object.prototype), {
    handleEvent: {
      /** @type {function(!KeyboardEvent): boolean} */
      value: function(event) {
        var code = event.code;
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
    hasValue: { value: false },
    number: { value: 0 },
    numDigits: { value: 0 },
    reset: {
      /** @type{function()} */
      value: function() {
        this.hasValue = false;
        this.number_ = 0;
        this.numDigits_ = 0;
        this.sign_ = 0;
      }
    },
    /** @private*/ sign: { value: 0 },
    value: {
      /** @type{function():number} */
      get: function() {
        if (!this.hasValue)
          return 1;
        if (this.numDigits_)
          return this.number_ * this.sign_;
        return this.sign_ ? -4 : 4;
      }
    }
  }));

  /** @type{!function(!KeyboardEvent)} */
  var handler;

  /** @type{?Map} */
  var keymap;

  /** @type{Array.<number>} */
  var key_codes = [];

  /**
   * @param {!Event} event
   * @return {?Window}
   */
  function eventWindow(event) {
    return event.target instanceof Window ?
        /** @type{!Window} */(event.target) : Editor.activeWindow();
  }

  /**
   * @param {!KeyboardEvent} event
   * @return {!Function|!Map|undefined}
   */
  function getKeyBinding(event) {
    var code = event.code;

    function get(map) {
      return map instanceof Map ? map.get(code) : undefined;
    }

    return get(keymap) || get(event.target['keyBindingMap']) ||
        get(event.target.constructor['keyBindingMap']);
  }

  /** @param {!Event} event */
  function handleEvent(event) {
console.log('CommandProcessor.handleEvent', event);
    if (event instanceof KeyboardEvent)
      handleKeyboardEvent(event);
  }

  /** @param {!KeyboardEvent} event */
  function handleEventAsArgument(event) {
    argument.handleEvent(event) || handleEventAsCommand(event);
  }

  /** @param {!KeyboardEvent} event */
  function handleEventAsCommand(event) {
    key_codes.push(event.code);
    var binding = getKeyBinding(event);
    if (!binding) {
      handleUnboundKeySequence(event);
      return;
    }

    if (typeof(binding) == 'function') {
      if (argument.hasValue)
        binding.call(event.target, argument.value);
      else
        binding.call(event.target);
      reset();
      return;
    }

    if (binding instanceof Map) {
      keymap = /** @type{!Map} */(binding);
      return;
    }

    handleUnboundKeySequence(event);
  }

  /** @param {!KeyboardEvent} event */
  function handleEventAsQuote(event) {
    var char_code = event.code;
    if (char_code >= 0x80)
      return;
    var window = /** @type{TextWindow} */(event.target);
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
    /** @const @type{number} */ var MOD_CTRL = 0x200;
    /** @const @type{number} */ var MOD_SHIFT = 0x400;
    return ((key_code & MOD_CTRL) ? 'Ctrl+' : '') +
           ((key_code & MOD_SHIFT) ? 'Shift+' : '') +
           KEY_NAMES[key_code & 0x1FF];
  }

  return {
    handleEvent: { value: handleEvent },
    startArgument: { value: startArgument },
    startQuote: { value: startQuote }
  };
})()));
