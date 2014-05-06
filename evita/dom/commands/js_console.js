// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

var $0;

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
global.JsConsole = (function() {
  function JsConsole() {
    // TODO(yosi) We should make |*javascript*| document with JavaScript
    // syntax coloring.
    this.document = console.document;
    this.historyIndex = 0;
    this.history = [];
    this.lineNumber = 0;
    this.range = new Range(this.document);
    var self = /** @type {JsConsole} */(this);

    // JavaScript console specific key bindings.
    this.document.bindKey('Ctrl+ArrowDown', function() {
      self.forwardHistory();
      this.selection.range.collapseTo(self.range.end);
    });
    this.document.bindKey('Ctrl+L', function() {
      self.range.startOf(Unit.DOCUMENT);
      self.range.endOf(Unit.DOCUMENT, Alter.EXTEND);
      self.range.text = '';
      self.emitPrompt();
      this.selection.range.collapseTo(self.document.length);
    });
    this.document.bindKey('Ctrl+ArrowUp', function() {
      self.backwardHistory();
      this.selection.range.collapseTo(self.range.end);
    });
    this.document.bindKey('Enter', function() {
      this.selection.range.collapseTo(self.document.length);
      self.evalLastLine();
    });
  }
  return JsConsole;
})();

/** @type {number} */ JsConsole.MAX_HISTORY_LINES = 20;
/** @type {?JsConsole} */ JsConsole.instance = null;

global.JsConsole.errorHandler = function(reason) {
  JsConsole.instance.errorHandler(reason);
};

/**
 * @this {!JsConsole}
 * @param {Window} activeWindow
 */
global.JsConsole.prototype.activate = function(activeWindow) {
  if (!activeWindow) {
    this.newWindow();
    return;
  }

  var activeEditor_window = activeWindow instanceof EditorWindow ?
    activeWindow : activeWindow.parent;
  var document = this.document;
  var present = null;
  activeEditor_window.children.forEach(function(window) {
    if (window.document == document)
      present = window;
  });

  if (present) {
    present.selection.range.collapseTo(document.length);
    present.focus();
    return;
  }

  var newWindow = new TextWindow(new Range(document));
  activeEditor_window.appendChild(newWindow);
  newWindow.selection.range.collapseTo(document.length);
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
  if (this.historyIndex == this.history.length)
    return;
  ++this.historyIndex;
  this.useHistory();
};

/**
 * @param {string} text.
 */
JsConsole.prototype.emit = function(text) {
  this.document.readonly = false;
  //this.range.move(Unit.DOCUMENT, 1);
  this.range.collapseTo(this.document.length);
  this.range.insertBefore(text);
  this.document.readonly = true;
};

JsConsole.prototype.emitPrompt = function() {
  ++this.lineNumber;
  this.emit('\njs:' + this.lineNumber + '> ');
  // Off by one to keep |this.range.end| before user input.
  this.range.collapseTo(this.document.length - 1);
  this.document.readonly = false;
};

/**
 * @this {!JsConsole}
 * @param {*} reason
 */
global.JsConsole.prototype.errorHandler = function(reason) {
  $0 = reason;
  if (reason instanceof Error) {
    var stack = reason['stack'];
    if (stack) {
      this.emit('\x2F*\n' + stack + '\n*\x2F\n');
      return;
    }
    var message = reason['message'];
    if (message) {
      this.emit('\x2F*\nException: ' + message + '\n*\x2F\n');
      return;
    }
  }
  this.emit('JsConsole.errorHandler: ' + Editor.stringify(reason));
};

JsConsole.prototype.evalLastLine = function() {
  var range = this.range;
  range.end = this.document.length;
  var line = range.text.trim();
  if (line == '') {
    this.emitPrompt();
    return;
  }
  this.addToHistory_(line);
  this.historyIndex = 0;
  range.collapseTo(range.end);
  range.text = '\n';

  var result = Editor.runScript(line, console.DOCUMENT_NAME);
  JsConsole.result = result;
  range.collapseTo(range.end);
  if (result.exception) {
    $0 = result.exception;
    if (result.stackTraceString == '') {
      result.stackTraceString = result.exception +
        result.stackTrace.map(function(stackFrame) {
          return '\n  at ' + stackFrame.functionName + ' (' +
              stackFrame.scriptName + '(' + stackFrame.lineNumber + ':' +
              stackFrame.column + ')';
        }).join('');
    }
    this.emit('\x2F*\nException: ' + result.stackTraceString + '\n*\x2F\n');
  } else {
    if (result.value !== undefined)
      $0 = result.value;
    this.emit(Editor.stringify(result.value));
  }
  this.emitPrompt();
};

JsConsole.prototype.forwardHistory = function() {
  if (!this.historyIndex)
    return;
  --this.historyIndex;
  this.useHistory();
};

JsConsole.prototype.newWindow = function() {
  var editorWindow = new EditorWindow();
  var textWindow = new TextWindow(new Range(this.document));
  editorWindow.appendChild(textWindow);
  editorWindow.realize();
};

JsConsole.prototype.useHistory = function() {
  var range = this.range;
  range.end = this.document.length;
  range.text = this.history[this.history.length - this.historyIndex];
};

(function() {
  /**
   * Switch to JavaScript command.
   * @this {Window}
   */
  function switchToJsConsoleCommand() {
    var activeWindow = this.selection.window;
    if (JsConsole.instance) {
      JsConsole.instance.activate(activeWindow);
      return;
    }
    var instance = new JsConsole();
    JsConsole.instance = instance;
    instance.emit('\x2F/ JavaScript Console\n');
    instance.emitPrompt();
    instance.activate(activeWindow);
  }

  Editor.bindKey(Window, 'Ctrl+Shift+I', switchToJsConsoleCommand);
  Editor.bindKey(Window, 'Ctrl+Shift+J', switchToJsConsoleCommand);
})();
