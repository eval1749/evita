// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview Types
 * @externs
 */

/**
 * @type {Object.<string>}
 */
var global = {};

/**
 * @constructor
 */
var Document = function(name) {};

/**
 * @constructor
 */
var Editor = function() {};

/**
 * @interface
 */
var EventTarget = function() {};

/**
 * @constructor
 * @implements {EventTarget}
 * @param {number} resouce_id
 */
var FormControl = function(resouce_id) {};

/**
 * @constructor
 * @implements {EventTarget}
 */
var Window = function() {};

/**
 * @constructor
 * @extends {Window}
 */
var DocumentWindow = function() {};

/**
 * @constructor
 * @extends {Window}
 */
var EditorWindow = function() {};

/**
 * TODO(yosi) Until Closure compiler supports optional entries in @struct,
 * we use |Object|.
 * @typedef {Object}
 *   bubbles: boolean
 *   cancelable: boolean
 */
var EventInit;

/**
 * @constructor
 * @param {string} type
 * @param {EventInit=} opt_init_dict
 */
var Event = function(type, opt_init_dict) {};

/** @typedef {Function|Object} Object which has |handleEvent(event)| */
var EventListener;

var FilePath;

/**
 * @constructor
 * @implements {EventTarget}
 */
var Form = function() {};

/**
 * @constructor
 */
var JsConsole = function() {};

/**
 * Note: Since, Closure compiler doesn't support recursive type, we use
 * |!Map| instead of |!Keymap|.
 * @typedef {Map.<number, !Function|!Map>}
 */
var Keymap;

/**
 * @constructor
 * @param {number} x
 * @param {number} y
 */
var Point = function(x, y) {};

/** @typedef {number} */
var TextOffset;

/**
 * @constructor
 * @param {!Document} document
 * @param {!TextOffset} offset
 */
var TextPosition = function(document, offset) {};

/**
 * @constructor
 * @param {!Document|!Range} document_or_range
 * @param {number=} opt_start
 * @param {number=} opt_end
 */
var Range = function(document_or_range, opt_start, opt_end) {};

/**
 * @interface
 */
var Selection = function() {};

/**
 * @constructor
 */
var Style = function() {};

/**
 * @constructor
 * @extends {DocumentWindow}
 * @param {Document} document
 */
var TableWindow = function(document) {};

/**
 * @constructor
 * @extends {FormControl}
 * @param {number} resouce_id
 */
var TextFieldControl = function(resouce_id) {};

/**
 * @constructor
 * @extends {DocumentWindow}
 * @param {Range} range
 */
var TextWindow = function(range) {};

/**
 * @constructor
 * @implements {Selection}
 * You can't create |TableSelection| object dirctory. You can obtain
 * |TableSelection| object via |TextWindow.prototype.selection|.
 */
var TableSelection = function() {};

/**
 * @constructor
 * @implements {Selection}
 * You can't create |TextSelection| object dirctory. You can obtain
 * |TextSelection| object via |TextWindow.prototype.selection|.
 */
var TextSelection = function() {};

/**
 * TODO(yosi) Until Closure compiler supports optional entries in @struct,
 * we use |Object|.
 * @typedef {Object}
 *   bubbles: boolean
 *   cancelable: boolean
 *   detail: number
 *   view: ?Window
 */
var UiEventInit;

/**
 * @constructor
 * @extends {Event}
 * @param {string} type
 * @param {UiEventInit=} opt_init_dict
 */
var UiEvent = function(type, opt_init_dict) {};

/**
 * TODO(yosi) Until Closure compiler supports optional entries in @struct,
 * we use |Object|.
 * @typedef {Object}
 *   bubbles: boolean
 *   cancelable: boolean
 *   detail: number
 *   view: ?Window
 *   relatedTarget: ?Window
 */
var FocusEventInit;

/**
 * @constructor
 * @extends {UiEvent}
 * @param {string} type
 * @param {FocusEventInit=} opt_init_dict
 */
var FocusEvent = function(type, opt_init_dict) {};

/**
 * TODO(yosi) Until Closure compiler supports optional entries in @struct,
 * we use |Object|.
 * @typedef {Object}
 *   bubbles: boolean
 *   cancelable: boolean
 *   data: string
 */
var FormEventInit;

/**
 * @constructor
 * @extends {Event}
 * @param {string} type
 * @param {FormEventInit=} opt_init_dict
 */
var FormEvent = function(type, opt_init_dict) {};

/**
 * TODO(yosi) Until Closure compiler supports optional entries in @struct,
 * we use |Object|.
 * @typedef {Object}
 *   bubbles: boolean
 *   cancelable: boolean
 *   detail: number
 *   view: ?Window
 *   altKey: boolean,
 *   ctrlKey: boolean,
 *   code: number,
 *   location: number,
 *   metaKey: boolean,
 *   repeat: boolean,
 *   shiftKey: boolean,
 */
var KeyboardEventInit;

/**
 * @final
 * @constructor
 * @extends {Event}
 * @param {string} type
 * @param {KeyboardEventInit=} opt_init_dict
 */
var KeyboardEvent = function(type, opt_init_dict) {};

/**
 * TODO(yosi) Until Closure compiler supports optional entries in @struct,
 * we use |Object|.
 * @typedef {Object}
 *   bubbles: boolean
 *   cancelable: boolean
 *   detail: number
 *   view: ?Window
 *   altKey: boolean,
 *   button: number,
 *   buttns: number,
 *   ctrlKey: boolean,
 *   metaKey: boolean,
 *   shiftKey: boolean,
 */
var MouseEventInit;

/**
 * @constructor
 * @extends {Event}
 * @param {string} type
 * @param {MouseEventInit=} opt_init_dict
 */
var MouseEvent = function(type, opt_init_dict) {};

/**
 * TODO(yosi) Until Closure compiler supports optional entries in @struct,
 * we use |Object|.
 * @typedef {Object}
 *   bubbles: boolean
 *   cancelable: boolean
 *   souceWindow: !Window
 */
var WindowEventInit;

/**
 * @constructor
 * @extends {Event}
 * @param {string} type
 * @param {WindowEventInit=} opt_init_dict
 */
var WindowEvent = function(type, opt_init_dict) {};

//////////////////////////////////////////////////////////////////////
//
// Timer
//
/**
 * @constructor
 */
var Timer = function() {};

/**
 * @constructor
 * @extends {Timer}
 */
var OneShotTimer = function() {};

/**
 * @constructor
 * @extends {Timer}
 */
var RepeatingTimer = function() {};

//////////////////////////////////////////////////////////////////////
//
// Unicode
//
var Unicode;

/** @const @type{Array.<string>} */
Unicode.CATEGORY_SHORT_NAMES;

/** @enum{string} */
Unicode.Category = {
  Cc: "Cc", Cf: "Cf", Cn: "Cn", Co: "Co", Cs: "Cs",
  Ll: "Ll", Lm: "Lm", Lo: "Lo", Lt: "Lt", Lu: "Lu", Mc: "Mc"
};

/** @const @type{Array.<{bidi: string, category: string}>} */
Unicode.UCD;
