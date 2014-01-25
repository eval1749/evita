// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/**
 * The JavaScript Console.
 *
 * This file introduce editor command |SwitchToJavaScriptConsole| bound to
 * |Ctrl+Shift+I| and |Ctrl+Shift+J|.
 *
 * JavaScript console is represented by the document named |*javascript*| with
 * following key bindings:
 *  Enter Executes the last line, between end of prompt to end of document, as
 *        JavaScript source code and inserts result value into the document.
 *  Ctrl+Down Forward history
 *  Ctrl+L Clear console.
 *  Ctlr+Up Backward history
 *
 * @constructor
 */
function JsConsole() {
  // TODO(yosi) We should make |*javascript*| document with JavaScript
  // syntax coloring.
  this.document = console.document_();
  this.history_index = 0;
  this.history = [];
  this.lineNumber = 0;
  this.range = new Range(this.document);
  var self = /** @type {JsConsole} */(this);

  // JavaScript console specific key bindings.
  this.document.bindKey('Ctrl+Down', function() {
    self.forwardHistory();
    this.selection.range.collapseTo(self.range.end);
  });
  this.document.bindKey('Ctrl+L', function() {
    self.range.startOf(Unit.DOCUMENT);
    self.range.endOf(Unit.DOCUMENT, Alter.EXTEND);
    self.range.text = '';
    self.emitPrompt();
    this.selection.range.collapseTo(self.range.end);
  });
  this.document.bindKey('Ctrl+Up', function() {
    self.backwardHistory();
    this.selection.range.collapseTo(self.range.end);
  });
  this.document.bindKey('Enter', function() {
    this.selection.range.collapseTo(self.document.length);
    self.evalLastLine();
  });
}

/** @type {number} */ JsConsole.MAX_HISTORY_LINES = 20;
/** @type {?JsConsole} */ JsConsole.instance = null;

/**
  * Stringify value.
  * @param{*} value
  * @param{number=} MAX_LEVEL Default is 10.
  * @param{number=} MAX_LENGTH Default is 10.
  * @return {string}
  */
JsConsole.stringify = function(value, MAX_LEVEL, MAX_LENGTH) {
  MAX_LEVEL = arguments.length >= 1 ? MAX_LEVEL : 10;
  MAX_LENGTH = arguments.length >= 2 ? MAX_LENGTH : 10;
  var visited_map = new Map();
  var num_of_labels = 0;

  /** @param {!Object} object @return {string|undefined} */
  function getObjectIdLikeThing(object) {
    var key = ['id', 'name'].find(function(key) {
      return object[key] !== undefined;
    });
    return key ? object[key] : undefined;
  }

  // Note: I'm not sure why |Array| objects created by C++ has a different
  // |Array| construct from |Array| objects created in JavaScript. It seems
  // global objects are different in C++ and JavaScript.
  // Note: For |Arary|, we use |Array.isArray()|.
  // See http://web.mit.edu/jwalden/www/isArray.html, this problem can happen
  // in web browser, e.g. |window| in main resouce and |window| in IFRAME are
  // different.
  function isInstanceOf(value, klass) {
    if (typeof(value) != 'object')
      return false;
    if (value instanceof klass)
      return true;
    var ctor = value.constructor;
    return ctor && ctor.name == klass.name;
  }

  /** @interface */
  function Visitor() {};

  /** @param {number} index */
  Visitor.prototype.visitArrayElement = function(index) {};

  /** @param {*} atom */
  Visitor.prototype.visitAtom = function(atom) {};

  /** @param {!Object} object */
  Visitor.prototype.visitConstructed = function(object) {};

  /** @param {!Date} date*/
  Visitor.prototype.visitDate = function(date) {};

  /** @param {!Object} object */
  Visitor.prototype.visitFirstTime = function(object) {};

  /** @param {!Function} fun */
  Visitor.prototype.visitFunction = function(fun) {};

  /** @param {*} key @param {number} index */
  Visitor.prototype.visitKey = function(key, index) {};

  /** @param {string} string */
  Visitor.prototype.visitString = function(string) {};

  /** @param {!Object} object */
  Visitor.prototype.visitVisited = function(object) {};

  /** @param {!Array} array */
  Visitor.prototype.startArray = function(array) {};

  /** @param {!Array} array @param {boolean} limited */
  Visitor.prototype.endArray = function(array, limited) {};

  /** @param {!Object} object */
  Visitor.prototype.startObject = function(object) {};

  /** @param {boolean} limited */
  Visitor.prototype.endObject = function(limited) {};

  /**
   * @param {*} value
   * @param {number} level
   * @param {!Visitor} visitor
   */
  function visit(value, level, visitor) {
    if (value === null)
      return visitor.visitAtom('null');

    if (value === undefined)
      return visitor.visitAtom('undefined');

    if (value !== value)
      return visitor.visitAtom('NaN');

    if (value === Infinity)
      return visitor.visitAtom('Infinity');

    if (value === -Infinity)
      return visitor.visitAtom('-Infinity');

    switch (typeof(value)) {
      case 'boolean':
        return visitor.visitAtom(value.toString());
      case 'function':
        return visitor.visitFunction(value);
      case 'number':
        return visitor.visitAtom(value.toString());
      case 'string':
        return visitor.visitString(value);
    }

    var object = /** @type{!Object} */(value);

    if (visited_map.has(object))
      return visitor.visitVisited(object);
    visited_map.set(object, 0);

    ++level;
    if (level > MAX_LEVEL)
      return visitor.visitAtom('#');

    visitor.visitFirstTime(object);
    visited_map.set(object, 0);

    if (Array.isArray(object)) {
      var array = /** @type{!Array} */(object);
      visitor.startArray(array);
      var length = Math.min(array.length, MAX_LENGTH);
      for (var index = 0; index < length; ++index) {
        visitor.visitArrayElement(index);
        visit(array[index], level, visitor);
      }
      return visitor.endArray(array, array.length >= MAX_LENGTH);
    }

    if (isInstanceOf(object, Date))
      return visitor.visitDate(/** @type{!Date} */(object));

    if (object.constructor.name != 'Object')
      return visitor.visitConstructed(object);

    var keys = Object.keys(object).sort(function(a, b) {
      return a.localeCompare(b);
    });

    visitor.startObject(object);
    var count = 0;
    keys.forEach(function(key) {
      if (count > MAX_LENGTH)
        return;
      visitor.visitKey(key, count);
      visit(object[key], level, visitor);
      ++count;
    });
    visitor.endObject(count > keys.length);
  }

  /**
   * @constructor
   * @implements {Visitor}
   */
  function Labeler() {
    var label_map = new Map();
    var doNothing = function() {};
    this.labelOf = function(value) {
      return label_map.get(value);
    };
    this.visitArrayElement = doNothing;
    this.visitAtom = doNothing;
    this.visitConstructed = doNothing;
    this.visitDate= doNothing;
    this.visitFirstTime = doNothing;
    this.visitFunction = doNothing;
    this.visitKey = doNothing;
    this.visitString = doNothing;
    this.visitVisited = function(value) {
      if (label_map.has(value))
        return;
      label_map.set(value, (label_map.size + 1).toString());
    };
    this.startArray = doNothing;
    this.endArray = doNothing;
    this.startObject = doNothing;
    this.endObject = doNothing;
  }

  /** @const @type{{9: string, 10: string, 13:string}} */
  var ESCAPE_MAP = {
    0x09: 't',
    0x0A: 'n',
    0x0D: 'r'
  };

  /**
   * @constructor
   * @implements {Visitor}
   * @param {Labeler} labeler
   */
  function Printer(labeler) {
    this.result = '';
    /** @param {...} var_args */
    this.emit = function(var_args) {
      this.result += Array.prototype.slice.call(arguments, 0).join('');
    };
    this.visitAtom = function(x) {
      this.emit(x);
    };
    this.visitConstructed = function(object) {
      var ctor = object.constructor;
      var id = getObjectIdLikeThing(object);
      if (id == undefined)
        this.emit('#{', ctor ? ctor.name : 'Object', '}');
      else
        this.emit('#{', ctor ? ctor.name : 'Object', ' ' , id, '}');
    };
    this.visitDate = function(date) {
      this.emit('#{Date ', date.toString(), '}');
    };
    this.visitFirstTime = function(object) {
      var label = labeler.labelOf(object);
      if (label)
        this.emit('#', label, '=');
    };
    this.visitFunction = function(fun) {
      this.emit('(', fun.toString(), ')');
    };
    this.visitKey = function(key, index) {
      if (index)
        this.emit(', ');
      this.emit(key.toString(), ': ');
    };
    this.visitString = function(str) {
      this.emit('"');
      for (var index = 0; index < str.length; ++index) {
        var code = str.charCodeAt(index);
        if (code < 0x20 || (code >= 0x7F && code <= 0x9F)) {
          var escape = ESCAPE_MAP[code];
          if (escape) {
            this.emit('\\', escape);
          } else {
            var hex = ('0' + code.toString(16)).substr(code >= 0x10 ? 1 : 0);
            this.emit('\\0x', hex);
          }
        } else if (code == 0x22) {
          this.emit('\\"');
        } else if (code == 0x5C) {
          this.emit('\\\\');
        } else {
          this.emit(String.fromCharCode(code));
        }
      }
      this.emit('"');
    };
    this.visitVisited = function(value) {
      var label = labeler.labelOf(value);
      this.emit('#', label, '#');
    };
    this.startArray = function() {
      this.emit('[');
    };
    this.endArray = function(array, limited) {
      this.emit(limited ? ', ...' + array.length + ']' : ']');
    };
    this.visitArrayElement = function(index) {
      if (index)
        this.emit(', ');
    };
    this.startObject = function() {
      this.emit('{');
    };
    this.endObject = function(limited) {
      this.emit(limited ? ', ...}' : '}');
    };
  }

  var labeler = new Labeler();
  visit(value, 0, labeler);
  visited_map.clear();
  var printer = new Printer(labeler);
  visit(value, 0, printer);
  return printer.result;
}

/**
 * @param {?Window} active_window
 */
JsConsole.prototype.activate = function(active_window) {
  if (!active_window) {
    this.newWindow();
    return;
  }

  var active_editor_window = active_window instanceof EditorWindow ?
    active_window : active_window.parent;
  var document = this.document;
  var present = null;
  active_editor_window.children.forEach(function(window) {
    if (window.document == document)
      present = window;
  });

  if (!present) {
    present = new TextWindow(new Range(document));
    active_editor_window.appendChild(/** @type{!Window} */(present));
  }
  present.selection.range.collapseTo(document.length);
  present.focus();
};

/**
 * @private
 * @param {string} line
 */
JsConsole.prototype.addToHistory_ = function(line) {
  if (this.history.length && this.history[this.history.length - 1] == line)
    return;
  if (this.history.length > JsConsole.MAX_HISTORY_LINES)
    this.history.shift();
  this.history.push(line);
};

JsConsole.prototype.backwardHistory = function() {
  if (this.history_index == this.history.length)
    return;
  ++this.history_index;
  this.useHistory();
};

/**
 * @param {string} text.
 */
JsConsole.prototype.emit = function(text) {
  this.document.readOnly = false;
  //this.range.move(Unit.DOCUMENT, 1);
  this.range.collapseTo(this.document.length);
  this.range.insertBefore(text);
  this.document.readOnly = true;
};

JsConsole.prototype.emitPrompt = function() {
  ++this.lineNumber;
  this.emit('\njs:' + this.lineNumber + '> ');
  this.range.collapseTo(this.document.length);
  this.document.readOnly = false;
};

JsConsole.prototype.evalLastLine = function() {
  var range = this.range;
  range.end = this.document.length;
  if (range.start == range.end) {
    this.emitPrompt();
    return;
  }
  var line = range.text;
  this.addToHistory_(line);
  this.history_index = 0;
  range.collapseTo(range.end);
  range.insertBefore('\n');

  var result = Editor.runScript(line);
  JsConsole.result = result;
  range.collapseTo(range.end);
  if (result.exception) {
    this.emit('Exception: ' + result.exception);
  } else {
    this.emit(JsConsole.stringify(result.value));
  }
  this.emitPrompt();
};

JsConsole.prototype.forwardHistory = function() {
  if (!this.history_index)
    return;
  --this.history_index;
  this.useHistory();
};

JsConsole.prototype.newWindow = function() {
  var editor_window = new EditorWindow();
  var text_window = new TextWindow(new Range(this.document));
  editor_window.appendChild(text_window);
  editor_window.realize();
};

JsConsole.prototype.useHistory = function() {
  var range = this.range;
  range.end = this.document.length;
  range.text = this.history[this.history.length - this.history_index];
};

/**
 * Switch to JavaScript command.
 * @this {Window}
 */
function switchToJsConsoleCommand() {
  var active_window = this.selection.window;
  if (JsConsole.instance) {
    JsConsole.instance.activate(active_window);
    return;
  }
  var instance = new JsConsole();
  JsConsole.instance = instance;
  instance.emit('\x2F/ JavaScript Console\n');
  instance.emitPrompt();
  instance.activate(active_window);
}

(function() {
  Editor.bindKey(Window, 'Ctrl+Shift+I', switchToJsConsoleCommand);
  Editor.bindKey(Window, 'Ctrl+Shift+J', switchToJsConsoleCommand);
})();
