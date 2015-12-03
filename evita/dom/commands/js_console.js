// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
  /** @const @type {number} */ const MAX_HISTORY_LINES = 20;

  function installKeyBindings(jsConsole) {
    /** @const @type {!Document} */ let document = jsConsole.document_;
    // JavaScript console specific key bindings.
    document.bindKey('Ctrl+ArrowDown', function() {
      jsConsole.forwardHistory();
      this.selection.range.collapseTo(jsConsole.document_.length);
    });
    document.bindKey('Ctrl+L', function() {
      const document = jsConsole.range_.document;
      const readonly = document.readonly;
      document.readonly = false;
      jsConsole.range_.startOf(Unit.DOCUMENT);
      jsConsole.range_.endOf(Unit.DOCUMENT, Alter.EXTEND);
      jsConsole.range_.text = '';
      document.readonly = readonly;
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

  //////////////////////////////////////////////////////////////////////
  //
  // History
  //
  class History {
    constructor() {
      /** @type {number} */
      this.index_ = 0;
      /** @type {!Array.<string>} */
      this.lines_ = [];
    }

    /** @return {string} */
    get current() {
      return this.lines_[this.lines_.length - this.index_];
    }

    /**
     * @param {string} line
     */
    add(line) {
      this.index_ = 0;
      if (!this.shouldAdd_(line))
        return;
      if (this.lines_.length > MAX_HISTORY_LINES)
        this.lines_.shift();
      this.lines_.push(line);
    }

    /** @return {boolean} */
    backward() {
      if (this.index_ === this.lines_.length)
        return false;
      ++this.index_;
      return true;
    }

    /** @return {boolean} */
    forward() {
      if (this.index_ === 0)
        return false;
      --this.index_;
      return true;
    }

    /**
     * @param {string} line
     * @return {boolean}
     */
    shouldAdd_(line) {
      const length = this.lines_.length;
      if (length === 0)
        return true;
      return this.lines_[length - 1] !== line;
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // JsConsole
  //
  class JsConsole {
    constructor() {
      /** @const @type {!Document} */
      let document = console.document;
      // TODO(eval1749): We should make |*javascript*| document with JavaScript
      // syntax coloring.
      /** @type {!Document} */
      this.document_ = document;
      // TODO(eval1749): We should annotate |history_| with |History| once
      // Closure compiler recognize class |History|.
      /** @const $type {!History} */
      this.history_ = new History();
      /** @type {number} */
      this.lineNumber_ = 0;
      /** @const @type {!Range} */
      this.range_ = new Range(document);
      installKeyBindings(this);
      Object.seal(this);
    }

    /**
     * @this {!JsConsole}
     * @param {Window} activeWindow
     */
    activate(activeWindow) {
      if (!activeWindow) {
        this.newConsoleWindow();
        return;
      }

      let activeEditorWindow = activeWindow instanceof EditorWindow ?
        activeWindow : activeWindow.parent;
      let document = this.document_;
      let present = activeEditorWindow.children.find(function(window) {
        return window.document === document
      });
      if (present) {
        present.selection.range.collapseTo(document.length);
        present.focus();
        return;
      }

      let newWindow = new TextWindow(new Range(document));
      activeEditorWindow.appendChild(newWindow);
      newWindow.selection.range.collapseTo(document.length);
    }

    /**
     * @this {!JsConsole}
     */
    backwardHistory() {
      if (!this.history_.backward())
        return;
      this.useHistory();
    }

    /**
     * @this {!JsConsole}
     * @param {string} text
     */
    emit(text) {
      this.document_.readonly = false;
      //this.range_.move(Unit.DOCUMENT, 1);
      this.range_.collapseTo(this.document_.length);
      this.range_.insertBefore(text);
      this.document_.readonly = true;
    }

    /**
     * @this {!JsConsole}
     */
    emitPrompt() {
      ++this.lineNumber_;
      this.emit('\njs:' + this.lineNumber_ + '> ');
      // Off by one to keep |this.range_.end| before user input.
      this.range_.collapseTo(this.document_.length - 1);
      this.document_.readonly = false;
    }

    /**
     * @this {!JsConsole}
     */
    evalLastLine() {
      let range = this.range_;
      range.end = this.document_.length;
      let line = range.text.trim();
      if (line === '') {
        this.emitPrompt();
        return;
      }
      this.history_.add(line);
      range.collapseTo(range.end);
      range.text = '\n';

      let result = Editor.runScript(line, console.document.name);
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
    }

    /**
     * @this {!JsConsole}
     */
    forwardHistory() {
      if (!this.history_.forward())
        return;
      this.useHistory();
    }

    /**
     * @this {!JsConsole}
     * @param {*} reason
     */
    handleError(reason) {
      $0 = reason;
      if (reason instanceof Error) {
        let stack = reason['stack'];
        if (stack) {
          this.emit('\x2F*\n' + stack + '\n*\x2F\n');
          return;
        }
        let message = reason['message'];
        if (message) {
          this.emit('\x2F*\nException: ' + message + '\n*\x2F\n');
          return;
        }
      }
      this.emit('JsConsole.errorHandler: ' + Editor.stringify(reason));
      this.emit('\n');
    }

    /**
     * @param {!Promise} promise
     * @param {*} reason
     */
    static handleRejectedPromise(promise, reason) {
      let jsConsole = ensureJsConsole();
      jsConsole.emit('\n\x2F/Unhandled promise rejection:\n');
      jsConsole.handleError(reason);
    }

    /**
     * @this {!JsConsole}
     */
    newConsoleWindow() {
      let editorWindow = new EditorWindow();
      let textWindow = new TextWindow(new Range(this.document_));
      editorWindow.appendChild(textWindow);
      editorWindow.realize();
    }

    /**
     * @this {!JsConsole}
     */
    useHistory() {
      this.range_.end = this.document_.length;
      this.range_.text = ' ' + this.history_.current;
    }
  }
  Object.freeze(JsConsole.prototype);
  Object.seal(JsConsole);

  /** @type {JsConsole} */
  let instance = null;

  /**
   * @return {!JsConsole}
   */
  function ensureJsConsole() {
    if (instance)
      return instance;
    instance = new JsConsole();
    instance.emit(`\x2F/ JavaScript Console ${Editor.version},` +
                  ` v8:${Editor.v8Version}\n`);
    instance.emitPrompt();
    return instance;
  }

  /**
   * Switch to JavaScript command.
   * @this {Window}
   */
  function switchToJsConsoleCommand() {
     ensureJsConsole().activate(this.selection.window);
  }

  Editor.bindKey(Window, 'Ctrl+Shift+I', switchToJsConsoleCommand);
  Editor.bindKey(Window, 'Ctrl+Shift+J', switchToJsConsoleCommand);

  return JsConsole;
})();
