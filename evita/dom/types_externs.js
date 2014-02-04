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
 * @constructor
 */
var Event = function() {};

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
 * @constructor
 * @extends{Event}
 */
var UiEvent = function() {};

/**
 * @constructor
 * @extends{UiEvent}
 * @param {string} type
 * @param {?EventTarget} related_target
 */
var FocusEvent = function(type, related_target) {};

/**
 * @constructor
 * @extends{Event}
 * @param {string} type
 * @param {boolean} bubbles
 * @param {boolean} cancelable
 * @param {string} data
 */
var FormEvent = function(type, bubbles, cancelable, data) {};

/**
 * @constructor
 * @extends{Event}
 * @param {string} type
 * @param {number} click_count
 */
var MouseEvent = function(type, click_count) {};

/**
 * @constructor
 * @extends{Event}
 * @param {string} type
 * @param {boolean} bubbles
 * @param {boolean} cancelable
 * @param {!Window} source_window
 */
var WindowEvent = function(type, bubbles, cancelable, source_window) {};

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
