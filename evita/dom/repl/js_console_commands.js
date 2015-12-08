// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// JavaScript console specific key bindings.
(function() {
  /**
   * @param {!Document} document
   * @param {!repl.JsConsole} instance
   */
  function installCommands(document, instance) {
    document.bindKey('Ctrl+ArrowDown', /** @this {!TextWindow} */ function() {
      instance.forwardHistory();
      this.selection.endOf(Unit.DOCUMENT);
    });

    document.bindKey('Ctrl+L', /** @this {!TextWindow} */ function() {
      console.clear();
      instance.emitPrompt();
      this.selection.endOf(Unit.DOCUMENT);
    });

    document.bindKey('Ctrl+ArrowUp', /** @this {!TextWindow} */ function() {
      instance.backwardHistory();
      this.selection.endOf(Unit.DOCUMENT);
    });

    document.bindKey('Enter', /** @this {!TextWindow} */ function() {
      this.selection.endOf(Unit.DOCUMENT);
      instance.evalLastLine();
    });
  }

  Document.addObserver((type, document) => {
    if (type !== 'and')
      return;
    /** @type {!repl.JsConsole} */
    const instance = new repl.JsConsole(document);
    document.properties.set(repl.JsConsole.name, instance);
    installCommands(document, instance);
  });

  /**
   * Switch to JavaScript command.
   * @this {Window}
   */
  function switchToJsConsoleCommand() {
     windows.activate(this.selection.window, console.document);
  }

  Editor.bindKey(Window, 'Ctrl+Shift+J', switchToJsConsoleCommand);
})();
