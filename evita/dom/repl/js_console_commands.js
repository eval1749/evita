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
      instance.evalLastLine();
      this.selection.endOf(Unit.DOCUMENT);
    });
  }

  // Install JsConsole specific commands when "*javascript*" document is
  // created.
  Document.addObserver((type, document) => {
    if (type !== 'add' || document.name != repl.Console.DOCUMENT_NAME)
      return;
    /** @type {!repl.JsConsole} */
    const instance = new repl.JsConsole(document);
    document.properties.set(repl.JsConsole.name, instance);
    installCommands(document, instance);
    $0 = instance;
  });

  /**
   * Switch to JavaScript command.
   * @this {Window}
   */
  function switchToJsConsoleCommand() {
     const document = console.document;
     const isFirstTime = document.listWindows().length === 0;
     const window = windows.activate(this.selection.window, document);
     if (!isFirstTime)
       return;
     const instance = document.properties.get(repl.JsConsole.name);
     console.freshLine();
     console.emit(`\x2F/ JavaScript Console ${Editor.version},` +
                  ` v8:${Editor.v8Version}\n\n`);
     instance.emitPrompt();
     window.selection.endOf(Unit.DOCUMENT);
  }

  Editor.bindKey(Window, 'Ctrl+Shift+J', switchToJsConsoleCommand);
})();
