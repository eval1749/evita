// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// JavaScript console specific key bindings.
(function() {
  const JsConsole = repl.JsConsole;

  JsConsole.bindKey('Ctrl+ArrowDown', /** @this {!TextWindow} */ function() {
    JsConsole.instance.forwardHistory();
    this.selection.endOf(Unit.DOCUMENT);
  });

  JsConsole.bindKey('Ctrl+L', /** @this {!TextWindow} */ function() {
    console.clear();
    JsConsole.instance.emitPrompt();
    this.selection.endOf(Unit.DOCUMENT);
  });

  JsConsole.bindKey('Ctrl+ArrowUp', /** @this {!TextWindow} */ function() {
    JsConsole.instance.backwardHistory();
    this.selection.endOf(Unit.DOCUMENT);
  });

  JsConsole.bindKey('Ctrl+I', /** @this {!TextWindow} */ function() {
    const selection = /** @type {!TextSelection} */ (this.selection);
    const completer = repl.JsCompleter.ensure(JsConsole.instance, selection);
    completer.perform();
  });

  JsConsole.bindKey('Enter', /** @this {!TextWindow} */ function() {
    JsConsole.instance.evalLastLine();
    this.selection.endOf(Unit.DOCUMENT);
  });

  /**
   * Switch to JavaScript command.
   * @this {Window}
   */
  function switchToJsConsoleCommand() {
    const document = console.document;
    const window = windows.activate(this, document);
    const commandLoop = JsConsole.instance;
    if (commandLoop.lineNumber === 0) {
      console.freshLine();
      console.emit(
          `\x2F/ JavaScript Console ${Editor.version},` +
          ` v8:${Editor.v8Version}\n\n`);
      commandLoop.emitPrompt();
    }
    window.selection.endOf(Unit.DOCUMENT);
  }

  Editor.bindKey(Window, 'Ctrl+Shift+J', switchToJsConsoleCommand);
})();
