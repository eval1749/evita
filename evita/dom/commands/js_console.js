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
 */
global.JsConsole = (function() {
  function installKeyBindings(jsConsole) {
    /** @const @type {!Document} */ var document = jsConsole.document_;
    // JavaScript console specific key bindings.
    document.bindKey('Ctrl+ArrowDown', function() {
      jsConsole.forwardHistory();
      this.selection.range.collapseTo(jsConsole.document_.length);
    });
    document.bindKey('Ctrl+L', function() {
      jsConsole.range_.startOf(Unit.DOCUMENT);
      jsConsole.range_.endOf(Unit.DOCUMENT, Alter.EXTEND);
      jsConsole.range_.text = '';
      jsConsole.emitPrompt();
      this.selection.range.collapseTo(jsConsole.document_.length);
    });
    document.bindKey('Ctrl+ArrowUp', function() {
      jsConsole.backwardHistory();
      this.selection.range.collapseTo(jsConsole.document_.length);
    });
    document.bindKey('Enter', function() {
      this.selection.range.collapseTo(jsConsole.document_.length);
      jsConsole.evalLastLine();
    });
  }

  /**
   * @constructor
   * @final
   * @struct
   */
  function JsConsole() {
    /** @const @type {!Document} */
    var document = console.document;
    // TODO(yosi) We should make |*javascript*| document with JavaScript
    // syntax coloring.
    /** @type {!Document} */
    this.document_ = document;
    /** @type {number} */
    this.historyIndex_ = 0;
    /** @type {!Array.<string>} */
    this.history_ = [];
    /** @type {number} */
    this.lineNumber_ = 0;
    /** @const @type {!Range} */
    this.range_ = new Range(document);
    installKeyBindings(this);
    Object.seal(this);
  }

  /** @const @type {number} */ var MAX_HISTORY_LINES = 20;
  /** @type {JsConsole} */ var instance = null;

  /**
   * @return {!JsConsole}
   */
  function ensureJsConsole() {
    if (instance)
      return instance;
    instance = new JsConsole();
    instance.emit('\x2F/ JavaScript Console\n');
    instance.emitPrompt();
    return instance;
  }

  Object.defineProperties(JsConsole, {
    errorHandler: {value: function(reason) {
      return errorHandler.call(ensureJsConsole(), reason);
    }}
  });

  /**
   * @this {!JsConsole}
   * @param {Window} activeWindow
   */
  function activate(activeWindow) {
    if (!activeWindow) {
      newConsoleWindow(this);
      return;
    }

    var activeEditor_window = activeWindow instanceof EditorWindow ?
      activeWindow : activeWindow.parent;
    var document = this.document_;
    var present = null;
    activeEditor_window.children.forEach(function(window) {
      if (window.document === document)
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
   * @param {!JsConsole} historyOwner
   * @param {string} line
   */
  function addToHistory(historyOwner, line) {
    var history = historyOwner.history_;
    if (history.length && history[history.length - 1] === line)
      return;
    if (history.length > MAX_HISTORY_LINES)
      history.shift();
    history.push(line);
  };

  /**
   * @this {!JsConsole}
   */
  function backwardHistory() {
    if (this.historyIndex_ === this.history_.length)
      return;
    ++this.historyIndex_;
    this.useHistory();
  };

  /**
   * @this {!JsConsole}
   * @param {string} text
   */
  function emit(text) {
    this.document_.readonly = false;
    //this.range_.move(Unit.DOCUMENT, 1);
    this.range_.collapseTo(this.document_.length);
    this.range_.insertBefore(text);
    this.document_.readonly = true;
  };

  /**
   * @this {!JsConsole}
   */
  function emitPrompt() {
    ++this.lineNumber_;
    this.emit('\njs:' + this.lineNumber_ + '> ');
    // Off by one to keep |this.range_.end| before user input.
    this.range_.collapseTo(this.document_.length - 1);
    this.document_.readonly = false;
  };

  /**
   * @this {!JsConsole}
   * @param {*} reason
   */
  function errorHandler(reason) {
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

  /**
   * @this {!JsConsole}
   */
  function evalLastLine() {
    var range = this.range_;
    range.end = this.document_.length;
    var line = range.text.trim();
    if (line === '') {
      this.emitPrompt();
      return;
    }
    addToHistory(this, line);
    this.historyIndex_ = 0;
    range.collapseTo(range.end);
    range.text = '\n';

    var result = Editor.runScript(line, console.DOCUMENT_NAME);
    range.collapseTo(range.end);
    if (result.exception) {
      $0 = result.exception;
      if (result.stackTraceString === '') {
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

  /**
   * @this {!JsConsole}
   */
  function forwardHistory() {
    if (this.historyIndex_ <= 1)
      return;
    --this.historyIndex_;
    this.useHistory();
  };

  /**
   * @param {!JsConsole} jsConsole
   */
  function newConsoleWindow(jsConsole) {
    var editorWindow = new EditorWindow();
    var textWindow = new TextWindow(new Range(jsConsole.document_));
    editorWindow.appendChild(textWindow);
    editorWindow.realize();
  };

  /**
   * @this {!JsConsole}
   */
  function useHistory() {
    var range = this.range_;
    range.end = this.document_.length;
    range.text = ' ' + this.history_[this.history_.length - this.historyIndex_];
  };

  /**
   * Switch to JavaScript command.
   * @this {Window}
   */
  function switchToJsConsoleCommand() {
     ensureJsConsole().activate(this.selection.window);
  }

  Editor.bindKey(Window, 'Ctrl+Shift+I', switchToJsConsoleCommand);
  Editor.bindKey(Window, 'Ctrl+Shift+J', switchToJsConsoleCommand);

  Object.defineProperties(JsConsole.prototype, {
    activate: {value: activate},
    backwardHistory: {value: backwardHistory},
    constructor: {value: JsConsole},
    emit: {value: emit},
    emitPrompt: {value: emitPrompt},
    errorHandler: {value: errorHandler},
    evalLastLine: {value: evalLastLine},
    useHistory: {value: useHistory}
  });
  Object.freeze(JsConsole.prototype);
  Object.seal(JsConsole);
  return JsConsole;
})();
