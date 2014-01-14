// Copyright (c) 2014 Project Vogue. All rights reserved.
  // Use of this source code is governed by a BSD-style license that can be
  // found in the LICENSE file.

  'use strict';
var JsConsole = (function() {
  /**
   * Stringify value.
   * @param{Object} value
   * @param{number} MAX_LEVEL. Default is 10.
   * @param{number} MAX_LENGTH. Default is 10.
   * @return {string}
   */
  function stringify(value, MAX_LEVEL, MAX_LENGTH) {
    MAX_LEVEL = arguments.length >= 1 ? MAX_LEVEL : 10;
    MAX_LENGTH = arguments.length >= 2 ? MAX_LENGTH : 10;
    var visited_map = new Map();
    var num_of_labels = 0;

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

      if (visited_map.has(value))
        return visitor.visitVisited(value);
      visited_map.set(value, 0);

      ++level;
      if (level > MAX_LEVEL)
        return visitor.visitAtom('#');

      visitor.visitFirstTime(value);
      visited_map.set(value, 0);

      if (value instanceof Array) {
        visitor.startArray(value);
        var length = Math.min(value.length, MAX_LENGTH);
        for (var index = 0; index < length; ++index) {
          visitor.visitArrayElement(index);
          visit(value[index], level, visitor);
        }
        return visitor.endArray(value.length >= MAX_LENGTH);
      }

      if (value instanceof Date)
        return visitor.visitDate(value);

      if (value.constructor != Object)
        return visitor.visitConstructed(value);

      var keys = Object.keys(value).sort(function(a, b) {
        return a.localeCompare(b);
      });

      visitor.startObject(value);
      var count = 0;
      keys.forEach(function(key) {
        if (count > MAX_LENGTH)
          return;
        visitor.visitKey(key, count);
        visit(value[key], level, visitor);
        ++count;
      });
      visitor.endObject(count > keys.length);
    }

    function Labeler() {
      var label_map = new Map();
      var doNothing = function() {};
      this.labelOf = function(value) {
        return label_map.get(value);
      };
      this.visitArrayElement = doNothing;
      this.visitAtom = doNothing;
      this.visitConstructed = doNothing;
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

    var ESCAPE_MAP = {
      0x09: 't',
      0x0A: 'n',
      0x0D: 'r'
    };

    function Printer(labeler) {
      this.result = '';
      this.emit = function() {
        this.result += Array.prototype.slice.call(arguments, 0).join('');
      };
      this.visitAtom = function(x) {
        this.emit(x);
      };
      this.visitConstructed = function(object) {
        var ctor = object.constructor;
        this.emit('#{', ctor ? ctor.name : 'Object', '}');
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
      this.endArray = function(limited) {
        this.emit(limited ? ', ...]' : ']');
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

  function JsConsole() {
    this.document = console.document_();
    this.range = new Range(this.document);
    this.lines = [];
    this.document.bindKey('Enter', evalLastLineCommand);
  }

  JsConsole.MAX_HISTORY_LINES = 20;
  JsConsole.instance = null;

  /**
   * @this {JsConsole}
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
      active_editor_window.add(present);
    }
    present.selection.range.collapseTo(document.length);
    present.focus();
  };

  /**
   * @this {JsConsole}
   * @param {string} text.
   */
  JsConsole.prototype.emit = function(text) {
    this.document.readOnly = false;
    //this.range.move(Unit.DOCUMENT, 1);
    this.range.collapseTo(this.document.length);
    this.range.insertBefore(text);
    this.document.readOnly = true;
  };

  /**
   * @this {JsConsole}
   */
  JsConsole.prototype.emitPrompt = function() {
    this.emit('\n> ');
    this.range.collapseTo(this.document.length);
    this.document.readOnly = false;
  };

  /**
   * @this {JsConsole}
   */
  JsConsole.prototype.evalLastLine = function() {
    var range = this.range;
    range.end = this.document.length;
    var line = range.text;
    if (this.lines.length > JsConsole.MAX_HISTORY_LINES)
      this.lines.shift();
    this.lines.push(line);
    range.collapseTo(range.end);
    range.insertBefore('\n');
    var result = Editor.runScript(line);
    JsConsole.result = result;
    range.collapseTo(range.end);
    if (result.exception) {
      this.emit('Exception: ' + result.exception.message);
    } else {
      this.emit(stringify(result.value));
    }
    this.emitPrompt();
  };

  /**
   * @this {JsConsole}
   */
  JsConsole.prototype.newWindow = function() {
    var editor_window = new EditorWindow();
    var text_window = new TextWindow(new Range(this.document));
    editor_window.add(text_window);
    editor_window.realize();
  };

  /**
   * Evaluate last line command.
   * @this {TextWindow}
   */
  function evalLastLineCommand() {
    JsConsole.instance.evalLastLine();
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
    var instance = new JsConsole(JsConsole.NAME);
    JsConsole.instance = instance;
    instance.emit('// JavaScript Console\n');
    instance.emitPrompt();
    instance.activate(active_window);
  }

  Editor.setKeyBinding('Ctrl+Shift+I', switchToJsConsoleCommand);

  return JsConsole;
})();
